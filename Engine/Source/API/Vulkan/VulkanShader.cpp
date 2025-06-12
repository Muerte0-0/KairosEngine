#include "kepch.h"
#include "VulkanShader.h"

#include "Engine/Core/Application.h"
#include "VulkanContext.h"
#include "VulkanUtils.h"

#include "Components/Command.h"
#include "Components/Frame.h"

#include "volk.h"

namespace Kairos
{
	PipelineLayoutBuilder::PipelineLayoutBuilder(VkDevice& logicalDevice) : m_LogicalDevice(logicalDevice) {}

	VkPipelineLayout PipelineLayoutBuilder::Build(std::deque<std::function<void(VkDevice)>>& deletionQueue)
	{
		VkPipelineLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		layoutInfo.setLayoutCount = (uint32_t)m_DescriptorLayouts.size();
		layoutInfo.pSetLayouts = m_DescriptorLayouts.data();

		VkPipelineLayout layout;
		VK_CHECK(vkCreatePipelineLayout(m_LogicalDevice, &layoutInfo, nullptr, &layout));

		Reset();

		deletionQueue.push_back([layout](VkDevice device)
			{
				vkDestroyPipelineLayout(device, layout, nullptr);
				KE_CORE_INFO("Destroyed Pipeline Layout");
			});

		return layout;
	}

	void PipelineLayoutBuilder::Reset()
	{
		m_DescriptorLayouts.clear();
	}

	shaderc_shader_kind ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")
			return shaderc_glsl_vertex_shader;
		if (type == "fragment" || type == "pixel")
			return shaderc_glsl_fragment_shader;

		KE_CORE_ASSERT(false, "Unknown Shader Type!");
		return shaderc_glsl_vertex_shader;
	}

	VulkanShader::VulkanShader(const std::string& filepath)
	{
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		shaderc::CompileOptions options;

		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
		options.SetSourceLanguage(shaderc_source_language_glsl);
		options.SetTargetSpirv(shaderc_spirv_version_1_6);
		options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::string source = ReadFile(filepath);
		auto sources = PreProcess(source);

		std::unordered_map<shaderc_shader_kind, std::vector<char>> shaderSources;

		for (auto source : sources)
		{
			shaderSources[source.first] = { source.second.begin(), source.second.end() };
		}

		// Extract Name From File Path //
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		m_ShaderName = filepath.substr(lastSlash, count);

		PreProcessShader(shaderSources, options);

		m_Shaders = MakeShaderObjects(vctx->GetVkContext().LogicalDevice, m_ShaderName.c_str(), shaderSources.at(shaderc_vertex_shader), shaderSources.at(shaderc_fragment_shader),
										vctx->GetDeviceDeletionQueue(), true);
	}

	VulkanShader::VulkanShader(const std::string& vertFilepath, const std::string& fragFilepath)
	{
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		// Extract Name From File Path //
		auto lastSlash = vertFilepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = vertFilepath.rfind('.');
		auto count = lastDot == std::string::npos ? vertFilepath.size() - lastSlash : lastDot - lastSlash;
		m_ShaderName = vertFilepath.substr(lastSlash, count);

		auto vertSrc = ReadCode(vertFilepath + ".spv");
		auto fragSrc = ReadCode(fragFilepath + ".spv");

		m_Shaders = MakeShaderObjects(vctx->GetVkContext().LogicalDevice, m_ShaderName.c_str(), vertSrc, fragSrc,
			vctx->GetDeviceDeletionQueue(), false);
	}

	VulkanShader::VulkanShader(std::string& shaderName, std::string& vertCode, std::string& fragCode) : m_ShaderName(shaderName)
	{
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		shaderc::CompileOptions options;

		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
		options.SetSourceLanguage(shaderc_source_language_glsl);
		options.SetTargetSpirv(shaderc_spirv_version_1_6);
		options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::unordered_map<shaderc_shader_kind, std::vector<char>> shaderSources;

		std::vector<char> vertSrc(vertCode.begin(), vertCode.end());
		std::vector<char> fragSrc(fragCode.begin(), fragCode.end());

		shaderSources[shaderc_vertex_shader] = vertSrc;
		shaderSources[shaderc_fragment_shader] = fragSrc;

		PreProcessShader(shaderSources, options);

		m_Shaders = MakeShaderObjects(vctx->GetVkContext().LogicalDevice, m_ShaderName.c_str(), shaderSources.at(shaderc_vertex_shader), shaderSources.at(shaderc_fragment_shader),
			vctx->GetDeviceDeletionQueue(), true);
	}

	std::vector<VkShaderEXT> VulkanShader::MakeShaderObjects(VkDevice device, const char* name, std::vector<char> vertSrc, std::vector<char> fragSrc,
		std::deque<std::function<void(VkDevice)>>& deviceDeletionQueue, bool compileCode)
	{
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		VkShaderCreateFlagsEXT flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
		VkShaderStageFlags nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::vector<uint32_t> vertexCode;
		std::vector<uint32_t> fragmentCode;

		if (compileCode)
		{
			shaderc::CompileOptions options;

			options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
			options.SetSourceLanguage(shaderc_source_language_glsl);
			options.SetOptimizationLevel(shaderc_optimization_level_performance);
			options.SetForcedVersionProfile(450, shaderc_profile_core);

			vertexCode = Compile(vertSrc, options, shaderc_glsl_vertex_shader);
			fragmentCode = Compile(fragSrc, options, shaderc_glsl_fragment_shader);
		}

		VkShaderCodeTypeEXT codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;

		VkShaderCreateInfoEXT vertexInfo = {};
		vertexInfo.sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT;
		vertexInfo.flags  = flags;
		vertexInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertexInfo.nextStage = nextStage;
		vertexInfo.codeType = codeType;
		vertexInfo.setLayoutCount = 1;
		vertexInfo.pSetLayouts = &vctx->GetVkContext().DescriptorSetLayout;

		if (compileCode)
		{
			vertexInfo.codeSize = sizeof(uint32_t) * vertexCode.size();
			vertexInfo.pCode = vertexCode.data();
		}
		else
		{
			vertexInfo.codeSize = vertSrc.size();
			vertexInfo.pCode = vertSrc.data();
		}

		vertexInfo.pName  = "main";

		VkShaderCreateInfoEXT fragmentInfo = vertexInfo;
		fragmentInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragmentInfo.nextStage = 0;

		if (compileCode)
		{
			fragmentInfo.codeSize = sizeof(uint32_t) * fragmentCode.size();
			fragmentInfo.pCode = fragmentCode.data();
		}
		else
		{
			fragmentInfo.codeSize = fragSrc.size();
			fragmentInfo.pCode = fragSrc.data();
		}
		fragmentInfo.pName = "main";

		std::vector<VkShaderCreateInfoEXT> shaderInfos(2);
		shaderInfos[0] = vertexInfo;
		shaderInfos[1] = fragmentInfo;

		VkShaderEXT shaders[2];

		if (vkCreateShadersEXT(device, (uint32_t)shaderInfos.size(), shaderInfos.data(), nullptr, shaders) == VK_SUCCESS)
		{
			KE_CORE_INFO("{0} -> Shader Creation Success", m_ShaderName);
			
			VkShaderEXT vertexShader = shaders[0];
			deviceDeletionQueue.push_back([vertexShader](VkDevice device)
				{
					vkDestroyShaderEXT(device, vertexShader, nullptr);
				});

			VkShaderEXT fragmentShader = shaders[1];
			deviceDeletionQueue.push_back([fragmentShader](VkDevice device)
				{
					vkDestroyShaderEXT(device, fragmentShader, nullptr);
				});
		}
		else
			KE_CORE_ERROR("{0} -> Shader Creation Failed", m_ShaderName);

		std::vector<VkShaderEXT> vShaders =
		{
			shaders[0],
			shaders[1],
		};

		return vShaders;
	}

	std::vector<char> VulkanShader::ReadCode(const std::string& filepath)
	{
		std::ifstream file(filepath, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			KE_CORE_ERROR("Failed To Open File!");

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}

	std::string VulkanShader::ReadFile(const std::string& filepath)
	{
		std::string result;

		std::ifstream in(filepath, std::ios::in | std::ios::binary);

		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
		}
		else
			KE_CORE_ERROR("Could not Open File '{0}'", filepath);

		return result;
	}

	std::unordered_map<shaderc_shader_kind, std::string> VulkanShader::PreProcess(const std::string& source)
	{
		std::unordered_map<shaderc_shader_kind, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);

		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);
			KE_CORE_ASSERT(eol != std::string::npos, "Syntax Error");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);

			//KE_CORE_ASSERT(ShaderTypeFromString(type), "Invalid Shader Type Specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);
			shaderSources[ShaderTypeFromString(type)] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
		}

		return shaderSources;
	}

	void VulkanShader::PreProcessShader(std::unordered_map<shaderc_shader_kind, std::vector<char>>& shaderSources, shaderc::CompileOptions& options)
	{
		shaderc::Compiler compiler;

		for (auto& shaderSource : shaderSources)
		{
			shaderc::PreprocessedSourceCompilationResult result = compiler.PreprocessGlsl(shaderSource.second.data(), shaderSource.second.size(), shaderSource.first, m_ShaderName.c_str(), options);

			if (result.GetCompilationStatus() != shaderc_compilation_status_success)
				KE_CORE_ERROR(result.GetErrorMessage());

			const char* src = result.cbegin();
			size_t newSize = result.cend() - src;
			shaderSource.second.resize(newSize);
			memcpy(shaderSource.second.data(), src, newSize);
		}
	}

	std::vector<uint32_t> VulkanShader::Compile(std::vector<char>& shaderSource, shaderc::CompileOptions& options, shaderc_shader_kind kind)
	{
		shaderc::Compiler compiler;

		shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(
			shaderSource.data(), shaderSource.size(),
			kind,
			m_ShaderName.c_str(), "main", options);

		if (module.GetCompilationStatus() != shaderc_compilation_status_success)
			KE_CORE_ERROR("GLSL to SPIR-V Compilation : \n {0}", module.GetErrorMessage());

		return { module.cbegin(), module.cend() };
	}

	void VulkanShader::CreateDescriptorSet(uint32_t bindingIndex)
	{
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = vctx->GetVkContext().DescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &vctx->GetVkContext().DescriptorSetLayout;

		vkAllocateDescriptorSets(vctx->GetVkContext().LogicalDevice, &allocInfo, &m_DescriptorSet);

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_SceneDataBuffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(SceneData);

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_DescriptorSet;
		descriptorWrite.dstBinding = bindingIndex;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(vctx->GetVkContext().LogicalDevice, 1, &descriptorWrite, 0, nullptr);
	}

	void VulkanShader::UploadUniformInt(const std::string& name, int value)
	{
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		VkCommandBuffer cmd = BeginSingleTimeCommands(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().CommandPool);
		vkCmdPushConstants(cmd, vctx->GetVkContext().PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(value), &value);
		EndSingleTimeCommands(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().CommandPool, cmd, vctx->GetVkContext().GraphicsQueue);
	}

	void VulkanShader::UploadUniformFloat(const std::string & name, float value)
	{
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		VkCommandBuffer cmd = BeginSingleTimeCommands(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().CommandPool);
		vkCmdPushConstants(cmd, vctx->GetVkContext().PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(value), &value);
		EndSingleTimeCommands(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().CommandPool, cmd, vctx->GetVkContext().GraphicsQueue);
	}

	void VulkanShader::UploadUniformVec2(const std::string & name, const glm::vec2 & values)
	{
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		VkCommandBuffer cmd = BeginSingleTimeCommands(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().CommandPool);
		vkCmdPushConstants(cmd, vctx->GetVkContext().PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(values), &values);
		EndSingleTimeCommands(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().CommandPool, cmd, vctx->GetVkContext().GraphicsQueue);
	}

	void VulkanShader::UploadUniformVec3(const std::string & name, const glm::vec3 & values)
	{
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		VkCommandBuffer cmd = BeginSingleTimeCommands(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().CommandPool);
		vkCmdPushConstants(cmd, vctx->GetVkContext().PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(values), &values);
		EndSingleTimeCommands(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().CommandPool, cmd, vctx->GetVkContext().GraphicsQueue);
	}

	void VulkanShader::UploadUniformVec4(const std::string & name, const glm::vec4 & values)
	{
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		VkCommandBuffer cmd = BeginSingleTimeCommands(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().CommandPool);
		vkCmdPushConstants(cmd, vctx->GetVkContext().PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(values), &values);
		EndSingleTimeCommands(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().CommandPool, cmd, vctx->GetVkContext().GraphicsQueue);
	}

	void VulkanShader::UploadUniformMat3(const std::string & name, const glm::mat3 & matrix)
	{
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		VkCommandBuffer cmd = BeginSingleTimeCommands(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().CommandPool);
		vkCmdPushConstants(cmd, vctx->GetVkContext().PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(matrix), &matrix);
		EndSingleTimeCommands(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().CommandPool, cmd, vctx->GetVkContext().GraphicsQueue);
	}

	void VulkanShader::UploadUniformMat4(const std::string & name, const glm::mat4 & matrix)
	{
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		VkCommandBuffer cmd = BeginSingleTimeCommands(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().CommandPool);
		vkCmdPushConstants(cmd, vctx->GetVkContext().PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(matrix), &matrix);
		EndSingleTimeCommands(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().CommandPool, cmd, vctx->GetVkContext().GraphicsQueue);
	}

	void VulkanShader::UploadSceneData(const uint32_t& bindingIndex, const SceneData& sceneData)
	{
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		if (m_SceneDataBuffer == VK_NULL_HANDLE)
		{
			CreateBuffer(vctx->GetVkContext().PhysicalDevice, vctx->GetVkContext().LogicalDevice, sizeof(SceneData),
						VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						m_SceneDataBuffer, m_SceneDataMemory);

			vctx->GetDeviceDeletionQueue().push_back([this](VkDevice device)
			{
				if (m_SceneDataBuffer)
				{
					vkDestroyBuffer(device, m_SceneDataBuffer, nullptr);
					vkFreeMemory(device, m_SceneDataMemory, nullptr);
				}
			});
		}

		if (m_DescriptorSet == VK_NULL_HANDLE)
			CreateDescriptorSet(bindingIndex);

		void* data;
		vkMapMemory(vctx->GetVkContext().LogicalDevice, m_SceneDataMemory, 0, sizeof(SceneData), 0, &data);
		memcpy(data, &sceneData, sizeof(SceneData));
		vkUnmapMemory(vctx->GetVkContext().LogicalDevice, m_SceneDataMemory);
	}
}
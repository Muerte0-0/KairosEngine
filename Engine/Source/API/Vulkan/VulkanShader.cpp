#include "kepch.h"
#include "VulkanShader.h"

#include "Engine/Core/Application.h"
#include "VulkanContext.h"

#include "Components/Command.h"

#include "volk.h"

namespace Kairos
{
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

		PreProcessShader(shaderSources, options);
		CompileToAssembly(shaderSources, options);

		// Extract Name From File Path //
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		m_ShaderName = filepath.substr(lastSlash, count);

		auto shaders = MakeShaderObjects(vctx->GetVkContext().LogicalDevice, m_ShaderName.c_str(), shaderSources.at(shaderc_vertex_shader), shaderSources.at(shaderc_fragment_shader),
										vctx->GetDeviceDeletionQueue(), true);

		for (uint32_t i = 0; i < vctx->GetVkContext().Swapchain.Info().Images.size(); ++i)
			vctx->GetVkContext().Swapchain.Info().Frames[i].SetCommandBuffer(AllocateCommandBuffer(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().CommandPool), shaders, vctx->GetVkContext().Swapchain.Info().Extent);
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

		auto shaders = MakeShaderObjects(vctx->GetVkContext().LogicalDevice, m_ShaderName.c_str(), vertSrc, fragSrc,
			vctx->GetDeviceDeletionQueue(), false);

		for (uint32_t i = 0; i < vctx->GetVkContext().Swapchain.Info().Images.size(); ++i)
			vctx->GetVkContext().Swapchain.Info().Frames[i].SetCommandBuffer(AllocateCommandBuffer(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().CommandPool), shaders, vctx->GetVkContext().Swapchain.Info().Extent);
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
		CompileToAssembly(shaderSources, options);

		auto shaders = MakeShaderObjects(vctx->GetVkContext().LogicalDevice, m_ShaderName.c_str(), shaderSources.at(shaderc_vertex_shader), shaderSources.at(shaderc_fragment_shader),
			vctx->GetDeviceDeletionQueue(), true);

		for (uint32_t i = 0; i < vctx->GetVkContext().Swapchain.Info().Images.size(); ++i)
			vctx->GetVkContext().Swapchain.Info().Frames[i].SetCommandBuffer(AllocateCommandBuffer(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().CommandPool), shaders, vctx->GetVkContext().Swapchain.Info().Extent);
	}

	std::vector<VkShaderEXT> VulkanShader::MakeShaderObjects(VkDevice device, const char* name, std::vector<char> vertSrc, std::vector<char> fragSrc,
		std::deque<std::function<void(VkDevice)>>& deviceDeletionQueue, bool compileCode)
	{
		VkShaderCreateFlagsEXT flags = VkShaderCreateFlagBitsEXT::VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
		VkShaderStageFlags nextStage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;

		std::vector<uint32_t> vertexCode;
		std::vector<uint32_t> fragmentCode;

		if (compileCode)
		{
			shaderc::CompileOptions options;

			options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
			options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
			options.SetSourceLanguage(shaderc_source_language_glsl);
			options.SetTargetSpirv(shaderc_spirv_version_1_6);
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

			vertexCode = Compile(vertSrc, options);
			fragmentCode = Compile(fragSrc, options);
		}

		VkShaderCodeTypeEXT codeType = VkShaderCodeTypeEXT::VK_SHADER_CODE_TYPE_SPIRV_EXT;

		VkShaderCreateInfoEXT vertexInfo = {};
		vertexInfo.sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT;
		vertexInfo.flags  = flags;
		vertexInfo.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
		vertexInfo.nextStage = nextStage;
		vertexInfo.codeType = codeType;

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

		VkShaderCreateInfoEXT fragmentInfo = {};
		fragmentInfo.sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT;
		fragmentInfo.flags = flags;
		fragmentInfo.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
		fragmentInfo.codeType = codeType;

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

		if (vkCreateShadersEXT(device, shaderInfos.size(), shaderInfos.data(), nullptr, shaders) == VK_SUCCESS)
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

	void VulkanShader::Bind() const
	{

	}

	void VulkanShader::UnBind() const
	{

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

	void VulkanShader::CompileToAssembly(std::unordered_map<shaderc_shader_kind, std::vector<char>>& shaderSources, shaderc::CompileOptions& options)
	{
		shaderc::Compiler compiler;

		for (auto& shaderSource : shaderSources)
		{
			shaderc::AssemblyCompilationResult result = compiler.CompileGlslToSpvAssembly(shaderSource.second.data(), shaderSource.second.size(), shaderSource.first, m_ShaderName.c_str(), options);

			if (result.GetCompilationStatus() != shaderc_compilation_status_success)
				KE_CORE_ERROR(result.GetErrorMessage());

			const char* src = result.cbegin();
			size_t newSize = result.cend() - src;
			shaderSource.second.resize(newSize);
			memcpy(shaderSource.second.data(), src, newSize);
		}
	}

	std::vector<uint32_t> VulkanShader::Compile(std::vector<char>& shaderSource, shaderc::CompileOptions& options)
	{
		shaderc::Compiler compiler;

		shaderc::SpvCompilationResult module = compiler.AssembleToSpv(shaderSource.data(), shaderSource.size(), options);

		if (module.GetCompilationStatus() != shaderc_compilation_status_success)
			KE_CORE_ERROR("Assemble To Binary Compilation : \n {0}", module.GetErrorMessage());

		const uint32_t* src = module.cbegin();
		size_t wordCount = module.cend() - src;
		std::vector<uint32_t> output(wordCount);
		memcpy(output.data(), src, wordCount * sizeof(uint32_t));

		return output;
	}

}
#include "kepch.h"
#include "VulkanShader.h"

#include "VulkanRenderAPI.h"
#include "Engine/Renderer/Renderer.h"

namespace Engine
{
	VulkanShader::VulkanShader(const ShaderDescriptor& descriptor)
		: Shader(descriptor)
	{
		// No Vulkan work here — call Init() explicitly.
	}

	VulkanShader::~VulkanShader()
	{
		Shutdown();
	}
	
	void VulkanShader::Init()
	{
		Shutdown(); // Safe no-op if not yet initialized.

		LOG(LogLevel::Info, "VulkanShader '{}': Initializing {} stage(s).", m_Name, m_Stages.size());

		for (auto& stageData : m_Stages | views::values)
		{
			LoadSpirvFromDisk(stageData);
			CreateModule(stageData);
		}
	}

	void VulkanShader::Shutdown()
	{
		m_Modules.clear(); // vk::raii handles VkShaderModule destruction.
	}

	// -----------------------------------------------------------------------
	// Private helpers
	// -----------------------------------------------------------------------

	void VulkanShader::LoadSpirvFromDisk(ShaderStageData& stageData) const
	{
		const std::filesystem::path fullPath = m_ShaderDirectory / stageData.Filepath;

		std::ifstream stream(fullPath, std::ios::binary | std::ios::ate);
		ASSERT(stream.is_open(),
			"VulkanShader '{}': cannot open SPIR-V file '{}'.",
			m_Name, fullPath.string());

		const std::streamsize fileSize = stream.tellg();
		ASSERT(fileSize > 0,
			"VulkanShader '{}': SPIR-V file is empty: '{}'.", m_Name, fullPath.string());
		ASSERT((fileSize % static_cast<std::streamsize>(sizeof(uint32_t))) == 0,
			"VulkanShader '{}': '{}' is not valid SPIR-V (size not divisible by 4).",
			m_Name, fullPath.string());

		stream.seekg(0, std::ios::beg);
		stageData.Spirv.resize(static_cast<size_t>(fileSize / sizeof(uint32_t)));
		stream.read(reinterpret_cast<char*>(stageData.Spirv.data()), fileSize);

		ASSERT(stream.good() || stream.eof(),
			"VulkanShader '{}': failed to read SPIR-V from '{}'.", m_Name, fullPath.string());

		LOG(LogLevel::Info, "VulkanShader '{}': loaded {} ({} bytes).",
			m_Name, fullPath.filename().string(), fileSize);
	}

	void VulkanShader::CreateModule(ShaderStageData& stageData)
	{
		ASSERT(!stageData.Spirv.empty(),
			"VulkanShader '{}': stage '{}' has no SPIR-V — call LoadSpirvFromDisk first.",
			m_Name, Shader::StageToString(stageData.Stage));

		auto* api = dynamic_cast<VulkanRenderAPI*>(Renderer::GetAPI());
		ASSERT(api, "VulkanShader: active RenderAPI is not VulkanRenderAPI.");

		vk::ShaderModuleCreateInfo info;
		info.codeSize = stageData.Spirv.size() * sizeof(uint32_t);
		info.pCode    = stageData.Spirv.data();

		m_Modules.emplace(stageData.Stage,
			vk::raii::ShaderModule(api->GetVulkanDevice()->GetDevice(), info));
	}

	// -----------------------------------------------------------------------
	// Public accessors
	// -----------------------------------------------------------------------

	const vk::raii::ShaderModule& VulkanShader::GetModule(ShaderStage stage) const
	{
		ASSERT(m_Modules.contains(stage),
			"VulkanShader '{}': no module for stage '{}'.",
			m_Name, Shader::StageToString(stage));
		return m_Modules.at(stage);
	}

	vk::ShaderStageFlagBits VulkanShader::ToVkStage(ShaderStage stage)
	{
		switch (stage)
		{
		case ShaderStage::Vertex:   return vk::ShaderStageFlagBits::eVertex;
		case ShaderStage::Fragment: return vk::ShaderStageFlagBits::eFragment;
		case ShaderStage::Compute:  return vk::ShaderStageFlagBits::eCompute;
		default:
			ASSERT(false, "VulkanShader::ToVkStage — unhandled ShaderStage.");
			return vk::ShaderStageFlagBits::eVertex;
		}
	}
}

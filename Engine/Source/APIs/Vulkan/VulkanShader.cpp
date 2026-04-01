#include "kepch.h"
#include "VulkanShader.h"

#include "VulkanRenderAPI.h"
#include "Engine/Renderer/Renderer.h"

namespace Engine
{
	namespace
	{
		vk::ShaderStageFlagBits ToVulkanShaderStage(ShaderStage stage)
		{
			switch (stage)
			{
			case ShaderStage::Vertex:   return vk::ShaderStageFlagBits::eVertex;
			case ShaderStage::Fragment: return vk::ShaderStageFlagBits::eFragment;
			case ShaderStage::Compute:  return vk::ShaderStageFlagBits::eCompute;
			default:
				throw std::runtime_error("VulkanShader: unsupported shader stage.");
			}
		}
	}

	VulkanShader::VulkanShader(ShaderCreateInfo createInfo, ShaderBinary shaderBinary)
		: Shader(std::move(createInfo)), m_ShaderBinary(std::move(shaderBinary))
	{
		KE_CORE_ASSERT(!m_ShaderBinary.Bytecode.empty(), "VulkanShader: bytecode is empty for '{}'", GetFilepath().string());

		// Retrieve the logical device from the active Vulkan backend.
		auto* api = dynamic_cast<VulkanRenderAPI*>(Renderer::GetAPI());
		KE_CORE_ASSERT(api, "VulkanShader: active RenderAPI is not VulkanRenderAPI");

		vk::ShaderModuleCreateInfo moduleInfo;
		moduleInfo.codeSize = m_ShaderBinary.Bytecode.size() * sizeof(uint32_t);
		moduleInfo.pCode    = m_ShaderBinary.Bytecode.data();

		m_ShaderModule = vk::raii::ShaderModule(api->GetVulkanDevice()->GetDevice(), moduleInfo);
	}

	vk::ShaderStageFlagBits VulkanShader::GetVulkanStage() const
	{
		return ToVulkanShaderStage(GetStage());
	}
}

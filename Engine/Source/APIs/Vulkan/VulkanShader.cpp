#include "kepch.h"
#include "VulkanShader.h"

#include "Components/VulkanDevice.h"

namespace Engine
{
	namespace
	{
		vk::ShaderStageFlagBits ToVulkanShaderStage(const ShaderStage stage)
		{
			switch (stage)
			{
			case ShaderStage::Vertex:
				return vk::ShaderStageFlagBits::eVertex;
			case ShaderStage::Fragment:
				return vk::ShaderStageFlagBits::eFragment;
			case ShaderStage::Compute:
				return vk::ShaderStageFlagBits::eCompute;
			default:
				throw std::runtime_error("Unsupported shader stage.");
			}
		}
	}

	VulkanShader::VulkanShader(VulkanDevice& device, ShaderCreateInfo createInfo, ShaderBinary shaderBinary)
		: Shader(std::move(createInfo)), m_Device(device), m_ShaderBinary(std::move(shaderBinary))
	{
		KE_CORE_ASSERT(!m_ShaderBinary.Bytecode.empty(), "Shader bytecode is empty for {}", GetFilepath().string());

		vk::ShaderModuleCreateInfo shaderModuleCreateInfo;
		shaderModuleCreateInfo.codeSize = m_ShaderBinary.Bytecode.size() * sizeof(uint32_t);
		shaderModuleCreateInfo.pCode = m_ShaderBinary.Bytecode.data();

		m_ShaderModule = vk::raii::ShaderModule(m_Device.GetDevice(), shaderModuleCreateInfo);
	}

	vk::ShaderStageFlagBits VulkanShader::GetVulkanStage() const
	{
		return ToVulkanShaderStage(GetStage());
	}
}

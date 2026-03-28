#pragma once

#include "Engine/Renderer/RHI/Shader.h"

#include <vulkan/vulkan_raii.hpp>

namespace Engine
{
	class VulkanDevice;

	class VulkanShader : public Shader
	{
	public:
		VulkanShader(VulkanDevice& device, ShaderCreateInfo createInfo, ShaderBinary shaderBinary);

		const ShaderBinary& GetBinary() const { return m_ShaderBinary; }
		const vk::raii::ShaderModule& GetShaderModule() const { return m_ShaderModule; }
		vk::ShaderStageFlagBits GetVulkanStage() const;

	private:
		VulkanDevice& m_Device;
		ShaderBinary m_ShaderBinary;
		vk::raii::ShaderModule m_ShaderModule = nullptr;
	};
}

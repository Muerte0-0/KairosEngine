#pragma once

#include "Engine/Renderer/RHI/Shader.h"

#include <vulkan/vulkan_raii.hpp>

namespace Engine
{
	class VulkanDevice;

	class VulkanShader final : public Shader
	{
	public:
		/**
		 * @brief Construct from pre-loaded SPIR-V bytecode.
		 *        The device is obtained from the active VulkanRenderAPI.
		 */
		VulkanShader(ShaderCreateInfo createInfo, ShaderBinary shaderBinary);

		[[nodiscard]] const ShaderBinary&          GetBinary()       const { return m_ShaderBinary; }
		[[nodiscard]] const vk::raii::ShaderModule& GetShaderModule() const { return m_ShaderModule; }
		[[nodiscard]] vk::ShaderStageFlagBits        GetVulkanStage()  const;

	private:
		ShaderBinary         m_ShaderBinary;
		vk::raii::ShaderModule m_ShaderModule = nullptr;
	};
}

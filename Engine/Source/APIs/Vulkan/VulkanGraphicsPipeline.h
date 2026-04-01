#pragma once

#include "Engine/Renderer/RHI/GraphicsPipeline.h"

#include "VulkanShader.h"

#include <vulkan/vulkan_raii.hpp>

namespace Engine
{
	class VulkanDevice;

	class VulkanGraphicsPipeline final : public GraphicsPipeline
	{
	public:
		/**
		 * @brief Build the full Vulkan pipeline from createInfo.
		 *        The device is obtained from the active VulkanRenderAPI.
		 */
		explicit VulkanGraphicsPipeline(GraphicsPipelineCreateInfo createInfo);

		[[nodiscard]] const vk::raii::Pipeline&            GetPipeline()           const { return m_Pipeline; }
		[[nodiscard]] const vk::raii::PipelineLayout&      GetPipelineLayout()     const { return m_PipelineLayout; }
		[[nodiscard]] const vk::raii::DescriptorSetLayout& GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }

	private:
		// Owning references — device lifetime exceeds pipeline lifetime.
		Scope<VulkanShader> m_VertexShader   = nullptr;
		Scope<VulkanShader> m_FragmentShader  = nullptr;

		vk::raii::PipelineCache       m_PipelineCache       = nullptr;
		vk::raii::DescriptorSetLayout m_DescriptorSetLayout = nullptr;
		vk::raii::PipelineLayout      m_PipelineLayout      = nullptr;
		vk::raii::Pipeline            m_Pipeline            = nullptr;

		[[nodiscard]] ShaderBinary LoadShaderBinary(const std::filesystem::path& filepath) const;

		void CreateShaders();
		void CreatePipelineCache();
		void CreatePipelineLayout();
		void CreatePipeline();
	};
}

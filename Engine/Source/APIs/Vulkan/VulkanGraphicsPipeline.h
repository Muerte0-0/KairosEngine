#pragma once

#include "Engine/Renderer/RHI/GraphicsPipeline.h"
#include "VulkanShader.h"

#include <vulkan/vulkan_raii.hpp>

namespace Engine
{
	// -----------------------------------------------------------------------
	// Vulkan Graphics Pipeline
	// -----------------------------------------------------------------------

	class VulkanGraphicsPipeline final : public GraphicsPipeline
	{
	public:
		VulkanGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo);
		~VulkanGraphicsPipeline() override;

		// ------------------------------------------------------------------
		// Accessors
		// ------------------------------------------------------------------
		[[nodiscard]] const vk::raii::Pipeline&            GetPipeline()            const { return m_Pipeline; }
		[[nodiscard]] const vk::raii::PipelineLayout&      GetPipelineLayout()      const { return m_PipelineLayout; }
		[[nodiscard]] const vk::raii::DescriptorSetLayout& GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }

	private:
		GraphicsPipelineCreateInfo		m_CreateInfo;
		
		vk::raii::PipelineCache			m_PipelineCache       = nullptr;
		vk::raii::DescriptorSetLayout	m_DescriptorSetLayout = nullptr;
		vk::raii::PipelineLayout		m_PipelineLayout      = nullptr;
		vk::raii::Pipeline				m_Pipeline            = nullptr;

		void CreatePipelineCache();
		void CreatePipelineLayout();
		void CreatePipeline();
	};
}

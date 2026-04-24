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
		[[nodiscard]] const vk::raii::Pipeline&						GetPipeline()				const { return m_Pipeline; }
		[[nodiscard]] const vk::raii::PipelineLayout&				GetPipelineLayout()			const { return m_PipelineLayout; }
		[[nodiscard]] const vk::raii::DescriptorPool&				GetDescriptorPool()			const { return m_DescriptorPool; }
		[[nodiscard]] const vk::raii::DescriptorSetLayout&			GetDescriptorSetLayout()	const { return m_DescriptorSetLayout; }
		[[nodiscard]] const std::vector<vk::raii::DescriptorSet>&	GetDescriptorSets()			const { return m_DescriptorSets; }
		[[nodiscard]] vk::ShaderStageFlags							GetPushConstantStages()		const { return m_PushStages; }
		void UpdateShadowMapDescriptor(uint32_t frameIndex, const vk::DescriptorImageInfo& imageInfo) const;

	private:
		GraphicsPipelineCreateInfo				m_CreateInfo;
		vk::ShaderStageFlags					m_PushStages			= vk::ShaderStageFlagBits::eVertex;
		
		vk::raii::PipelineCache					m_PipelineCache			= nullptr;
		vk::raii::DescriptorPool				m_DescriptorPool		= nullptr;
		vk::raii::DescriptorSetLayout			m_DescriptorSetLayout	= nullptr;
		std::vector<vk::raii::DescriptorSet>	m_DescriptorSets;
		vk::raii::PipelineLayout				m_PipelineLayout		= nullptr;
		vk::raii::Pipeline						m_Pipeline				= nullptr;
		
		void CreatePipelineCache();
		void CreatePipelineLayout();
		void CreatePipeline();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
	};
}

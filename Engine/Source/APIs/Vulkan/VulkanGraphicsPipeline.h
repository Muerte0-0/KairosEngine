#pragma once

#include "Engine/Renderer/RHI/GraphicsPipeline.h"

#include "VulkanShader.h"

namespace Engine
{
	class VulkanDevice;

	class VulkanGraphicsPipeline : public GraphicsPipeline
	{
	public:
		VulkanGraphicsPipeline(VulkanDevice& device, GraphicsPipelineCreateInfo createInfo);

		const vk::raii::Pipeline& GetPipeline() const { return m_Pipeline; }
		const vk::raii::PipelineLayout& GetPipelineLayout() const { return m_PipelineLayout; }
		const vk::raii::DescriptorSetLayout& GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }

	private:
		VulkanDevice& m_VulkanDevice;

		Scope<VulkanShader> m_VertexShader = nullptr;
		Scope<VulkanShader> m_FragmentShader = nullptr;

		vk::raii::PipelineCache m_PipelineCache = nullptr;
		vk::raii::DescriptorSetLayout m_DescriptorSetLayout = nullptr;
		vk::raii::PipelineLayout m_PipelineLayout = nullptr;
		vk::raii::Pipeline m_Pipeline = nullptr;

		ShaderBinary LoadShaderBinary(const std::filesystem::path& filepath) const;
		void CreateShaders();
		void CreatePipelineCache();
		void CreateDescriptorSetLayoutStub();
		void CreatePipelineLayout();
		void CreatePipeline();
	};
}

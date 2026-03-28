#pragma once

#include "Shader.h"

#include <filesystem>

#include <vulkan/vulkan_raii.hpp>

namespace Engine
{
	struct GraphicsPipelineCreateInfo
	{
		std::filesystem::path ShaderDirectory;
		ShaderCreateInfo VertexShader;
		ShaderCreateInfo FragmentShader;
		vk::Format ColorFormat = vk::Format::eUndefined;
		vk::Format DepthFormat = vk::Format::eUndefined;
		vk::SampleCountFlagBits SampleCount = vk::SampleCountFlagBits::e1;
	};

	class GraphicsPipeline
	{
	public:
		explicit GraphicsPipeline(GraphicsPipelineCreateInfo createInfo);
		virtual ~GraphicsPipeline() = default;

		const GraphicsPipelineCreateInfo& GetCreateInfo() const { return m_CreateInfo; }

	protected:
		GraphicsPipelineCreateInfo m_CreateInfo;
	};
}

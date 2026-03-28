#pragma once

#include "Shader.h"

#include <filesystem>

#include <vulkan/vulkan_raii.hpp>

namespace Engine
{
	struct PipelineConfig
	{
		vk::PrimitiveTopology Topology = vk::PrimitiveTopology::eTriangleList;
		vk::PolygonMode PolygonMode = vk::PolygonMode::eFill;
		vk::CullModeFlags CullMode = vk::CullModeFlagBits::eBack;
		vk::FrontFace FrontFace = vk::FrontFace::eCounterClockwise;
		bool DepthTest = true;
	};

	struct GraphicsPipelineCreateInfo
	{
		std::filesystem::path ShaderDirectory;
		ShaderCreateInfo VertexShader;
		ShaderCreateInfo FragmentShader;
		PipelineConfig Config;
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
		const PipelineConfig& GetConfig() const { return m_CreateInfo.Config; }

	protected:
		GraphicsPipelineCreateInfo m_CreateInfo;
	};
}

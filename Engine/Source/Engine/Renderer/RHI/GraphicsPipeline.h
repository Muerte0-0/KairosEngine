#pragma once

#include "Shader.h"

#include <filesystem>

#include "Engine/Renderer/RendererUtils.h"

namespace Engine
{	
	struct GraphicsPipelineCreateInfo
	{
		std::filesystem::path ShaderDirectory;
		
		ShaderCreateInfo VertexShader;
		ShaderCreateInfo FragmentShader;
		
		TextureFormat ColorFormat = TextureFormat::Undefined;
		TextureFormat DepthFormat = TextureFormat::Undefined;
		SampleCountBits SampleCount = SampleCountBits::s1;
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

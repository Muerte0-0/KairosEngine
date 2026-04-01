#pragma once

#include "Shader.h"

#include "Engine/Renderer/RendererUtils.h"

namespace Engine
{
	// -----------------------------------------------------------------------
	// GraphicsPipelineCreateInfo
	// -----------------------------------------------------------------------

	struct GraphicsPipelineCreateInfo
	{
		std::filesystem::path ShaderDirectory;

		ShaderCreateInfo VertexShader;
		ShaderCreateInfo FragmentShader;

		TextureFormat    ColorFormat  = TextureFormat::Undefined;
		TextureFormat    DepthFormat  = TextureFormat::Undefined;
		SampleCountBits  SampleCount  = SampleCountBits::s1;
	};

	// -----------------------------------------------------------------------
	// Graphics Pipeline
	// -----------------------------------------------------------------------

	class GraphicsPipeline
	{
	public:
		explicit GraphicsPipeline(GraphicsPipelineCreateInfo createInfo);
		virtual ~GraphicsPipeline() = default;

		// No copying — GPU resources are move-only.
		GraphicsPipeline(const GraphicsPipeline&)            = delete;
		GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

		[[nodiscard]] const GraphicsPipelineCreateInfo& GetCreateInfo() const { return m_CreateInfo; }

		/**
		 * @brief Factory — dispatches to the active RHI backend.
		 * @parameter createInfo All data needed to build the pipeline.
		 * @return Owning pointer to the concrete Graphics Pipeline implementation.
		 */
		[[nodiscard]] static Scope<GraphicsPipeline> Create(GraphicsPipelineCreateInfo createInfo);

	protected:
		GraphicsPipelineCreateInfo m_CreateInfo;
	};
}

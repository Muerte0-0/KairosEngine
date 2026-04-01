#pragma once

#include "Shader.h"
#include "Engine/Renderer/RendererUtils.h"
#include "Engine/Core/Base.h"

namespace Engine
{
	// -----------------------------------------------------------------------
	// Graphics Pipeline CreateInfo
	// -----------------------------------------------------------------------

	struct GraphicsPipelineCreateInfo
	{
		Ref<Shader>     Shader;										// Must be loaded + Init().
		TextureFormat   ColorFormat = TextureFormat::Undefined;
		TextureFormat   DepthFormat = TextureFormat::Undefined;
		SampleCountBits SampleCount = SampleCountBits::s1;
	};

	// -----------------------------------------------------------------------
	// Graphics Pipeline
	// -----------------------------------------------------------------------

	class GraphicsPipeline
	{
	public:
		virtual ~GraphicsPipeline() = default;

		GraphicsPipeline(const GraphicsPipeline&)            = delete;
		GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

		[[nodiscard]] const GraphicsPipelineCreateInfo& GetCreateInfo() const { return m_CreateInfo; }

		/**
		 * @brief Initialize all GPU pipeline objects.
		 *        Must be called once after Create() before rendering.
		 */
		virtual void Init() = 0;

		/**
		 * @brief Release all GPU resources. Safe to call multiple times.
		 */
		virtual void Shutdown() = 0;
		
		[[nodiscard]] static Scope<GraphicsPipeline> Create(GraphicsPipelineCreateInfo createInfo);

	protected:
		explicit GraphicsPipeline(GraphicsPipelineCreateInfo createInfo);

		GraphicsPipelineCreateInfo m_CreateInfo;
	};
}

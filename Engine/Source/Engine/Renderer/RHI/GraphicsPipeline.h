#pragma once

#include "Shader.h"
#include "Engine/Utils/RendererUtils.h"
#include "Engine/Core/Base.h"

namespace Engine
{
	// -----------------------------------------------------------------------
	// Graphics Pipeline Create Info
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
		
		[[nodiscard]] static Scope<GraphicsPipeline> Create(GraphicsPipelineCreateInfo createInfo);
	};
}

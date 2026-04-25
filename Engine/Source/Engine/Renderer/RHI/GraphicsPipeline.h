#pragma once

#include "Shader.h"
#include "Buffer.h"
#include "Engine/Utils/RendererUtils.h"
#include "Engine/Core/Base.h"

namespace Engine
{
	// -----------------------------------------------------------------------
	// Graphics Pipeline Create Info
	// -----------------------------------------------------------------------

	struct GraphicsPipelineCreateInfo
	{
		Ref<Shader>     Shader;								// Must be loaded + Init().
		BufferLayout    VertexLayout;						// Describes vertex input — no mesh reference needed.
		TextureFormat   ColorFormat = TextureFormat::Undefined;
		TextureFormat   DepthFormat = TextureFormat::Undefined;
		SampleCountBits SampleCount = SampleCountBits::s1;
		CullMode        CullMode    = CullMode::Back;
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

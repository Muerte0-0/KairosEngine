#pragma once

#include "Engine/Renderer/RenderAPI.h"

namespace Kairos
{
	class OpenGLRenderAPI : public RenderAPI
	{
		// Inherited via RenderAPI
		void Init() override;
		void SetViewport(GraphicsContext* ctx, uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		void DrawIndexed(const Ref<VertexArray>& vertexArray) override;

	};
}
#pragma once

#include "Engine/Renderer/RenderAPI.h"

namespace Kairos
{
	class OpenGLRenderAPI : public RenderAPI
	{
		// Inherited via RenderAPI
		void Init() override;
		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		void Clear() override;
		void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) override;

	};
}
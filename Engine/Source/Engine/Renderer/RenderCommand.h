#pragma once

#include "RenderAPI.h"

#include "VertexArray.h"

namespace Kairos
{

	class RenderCommand
	{
	private:
		static RenderAPI* CreateRenderAPI();
		static RenderAPI* s_RendererAPI;

	public:
		inline static void Init() { s_RendererAPI->Init(); }

		inline static void SetViewport(GraphicsContext* ctx, uint32_t x, uint32_t y, uint32_t width, uint32_t height) { s_RendererAPI->SetViewport(ctx, x, y, width, height); }
		
		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray) { s_RendererAPI->DrawIndexed(vertexArray); }

		inline static RenderAPI* GetRenderAPI() { return s_RendererAPI; }
	};

}
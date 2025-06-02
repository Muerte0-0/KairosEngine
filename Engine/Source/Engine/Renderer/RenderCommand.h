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

		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) { s_RendererAPI->SetViewport(x, y, width, height); }

		inline static void Clear() { s_RendererAPI->Clear(); }
		
		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) { s_RendererAPI->DrawIndexed(vertexArray, indexCount); }

		inline static RenderAPI* GetRenderAPI() { return s_RendererAPI; }
	};

}
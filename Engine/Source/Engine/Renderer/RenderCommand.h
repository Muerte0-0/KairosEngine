#pragma once

#include "RenderAPI.h"

#include "VertexArray.h"

namespace Kairos
{

	class RenderCommand
	{
	public:
		inline static void Init()
		{
			s_RendererAPI->Init();
		}

		inline static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}

		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}
		
		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray)
		{
			s_RendererAPI->DrawIndexed(vertexArray);
		}

		inline static RenderAPI* GetRenderAPI() { return s_RendererAPI; }

	private:
		static RenderAPI* CreateRenderAPI();
		static RenderAPI* s_RendererAPI;
	};

}
#pragma once

#include <glm/glm.hpp>

#include "VertexArray.h"

#include "GraphicsContext.h"

namespace Kairos
{
	class RenderAPI
	{
	public:
		enum class API
		{
			None = 0,
			OpenGL = 1,
			Vulkan = 2,
			DX3D = 3
		};

	public:
		virtual void Init() = 0;
		virtual void SetViewport(GraphicsContext* ctx, uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) = 0;

		inline static API GetAPI() { return s_API; }
	private:
		static API s_API;
	};
}
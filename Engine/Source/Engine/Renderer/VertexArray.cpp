#include "kepch.h"
#include "VertexArray.h"

#include "Renderer.h"

#include "API/OpenGL/OpenGLVertexArray.h"

namespace Kairos
{
	VertexArray* VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:				KE_CORE_ASSERT(false, "RendererAPI::None -> Vertex Array Not Implemented!"); return nullptr;
			case RenderAPI::API::OpenGL:			return new OpenGLVertexArray();
			case RenderAPI::API::Vulkan:			KE_CORE_ASSERT(false, "RendererAPI::Vulkan -> Vertex Array Not Implemented!"); return nullptr;
			#ifdef KE_PLATFORM_WINDOWS
			case RenderAPI::API::DX3D:				KE_CORE_ASSERT(false, "RendererAPI::DX3D -> Vertex Array Not Implemented!"); return nullptr;
			#endif // KE_PLATFORM_WINDOWS
		}

		KE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
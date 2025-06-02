#include "kepch.h"
#include "Buffer.h"

#include "Renderer.h"

#include "API/OpenGL/OpenGLBuffer.h"
#include "API/Vulkan/VulkanBuffer.h"

namespace Kairos
{
	VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:				KE_CORE_ASSERT(false, "RendererAPI::None -> Vertex Buffer Not Implemented!"); return nullptr;
			case RenderAPI::API::OpenGL:			return new OpenGLVertexBuffer(vertices, size);
			case RenderAPI::API::Vulkan:			return new VulkanVertexBuffer(vertices, size);
			#ifdef KE_PLATFORM_WINDOWS
			case RenderAPI::API::DX3D:				KE_CORE_ASSERT(false, "RendererAPI::DX3D -> Vertex Buffer Not Implemented!"); return nullptr;
			#endif // KE_PLATFORM_WINDOWS

		}

		KE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:				KE_CORE_ASSERT(false, "RendererAPI::None -> Index Buffer Not Implemented!"); return nullptr;
			case RenderAPI::API::OpenGL:			return new OpenGLIndexBuffer(indices, count);
			case RenderAPI::API::Vulkan:			return new VulkanIndexBuffer(indices, count);
			#ifdef KE_PLATFORM_WINDOWS
			case RenderAPI::API::DX3D:				KE_CORE_ASSERT(false, "RendererAPI::DX3D -> Index Buffer Not Implemented!"); return nullptr;
			#endif // KE_PLATFORM_WINDOWS

		}

		KE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
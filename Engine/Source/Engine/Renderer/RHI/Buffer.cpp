#include "kepch.h"
#include "Buffer.h"

#include "Engine/Renderer/Renderer.h"

#include "APIs/Vulkan/VulkanBuffer.h"

namespace Engine
{
	// -----------------------------------------------------------------------
	// Vertex Buffer
	// -----------------------------------------------------------------------

	Ref<VertexBuffer> VertexBuffer::Create(const void* data, uint32_t size, BufferUsage usage)
	{
		switch (Renderer::GetAPI()->GetType())
		{
		case API::Vulkan:
			return CreateRef<VulkanVertexBuffer>(data, size, usage);
#ifdef PLATFORM_WINDOWS
		case API::DX11:
			ASSERT(false, "API[Direct X 11]: Not Implemented");
			break;
		case API::DX12:
			ASSERT(false, "API[Direct X 12]: Not Implemented");
			break;
#endif
		}
		
		return nullptr;
	}

	// -----------------------------------------------------------------------
	// Index Buffer
	// -----------------------------------------------------------------------

	Ref<IndexBuffer> IndexBuffer::Create(const void* indices, uint32_t count, uint32_t indexSize, BufferUsage usage)
	{
		switch (Renderer::GetAPI()->GetType())
		{
		case API::Vulkan:
			return CreateRef<VulkanIndexBuffer>(indices, count, indexSize, usage);
#ifdef PLATFORM_WINDOWS
		case API::DX11:
			ASSERT(false, "API[Direct X 11]: Not Implemented");
			break;
		case API::DX12:
			ASSERT(false, "API[Direct X 12]: Not Implemented");
			break;
#endif
		}
		
		return nullptr;
	}
}

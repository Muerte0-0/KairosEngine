#include "kepch.h"
#include "Framebuffer.h"

#include "Engine/Renderer/Renderer.h"

#include "APIs/Vulkan/VulkanFramebuffer.h"

namespace Engine
{
	Scope<Framebuffer> Framebuffer::Create(uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI()->GetType())
		{
		case API::Vulkan:
			return CreateScope<VulkanFramebuffer>(width, height);
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

#include "kepch.h"
#include "Framebuffer.h"

#include "Engine/Renderer/Renderer.h"

#include "APIs/Vulkan/VulkanFramebuffer.h"

namespace Engine
{
	Scope<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		switch (Renderer::GetAPI()->GetType())
		{
		case API::Vulkan:
			return CreateScope<VulkanFramebuffer>(spec);
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

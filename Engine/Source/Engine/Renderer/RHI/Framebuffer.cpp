#include "kepch.h"
#include "Framebuffer.h"

#include "Engine/Renderer/Renderer.h"

#include "APIs/Vulkan/VulkanFramebuffer.h"
#include "APIs/Vulkan/VulkanRenderAPI.h"

namespace Engine
{
	Scope<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		switch (Renderer::GetAPI()->GetType())
		{
		case API::Vulkan:
		{
			auto* api = dynamic_cast<VulkanRenderAPI*>(Renderer::GetAPI());
			ASSERT(api, "Framebuffer::Create: active RenderAPI is not VulkanRenderAPI.");
			return CreateScope<VulkanFramebuffer>(*api->GetVulkanDevice(), spec);
		}
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

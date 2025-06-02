#include "kepch.h"
#include "Framebuffer.h"

#include "Renderer.h"
#include "API/OpenGL/OpenGLFramebuffer.h"
#include "API/Vulkan/VulkanFramebuffer.h"

namespace Kairos
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		switch (RenderAPI::GetAPI())
		{
		case RenderAPI::API::None:				KE_CORE_ASSERT(false, "RendererAPI::None -> Framebuffer Not Implemented!");
		case RenderAPI::API::OpenGL:			return CreateRef<OpenGLFramebuffer>(spec);
		case RenderAPI::API::Vulkan:			return CreateRef<VulkanFramebuffer>(spec);
#ifdef KE_PLATFORM_WINDOWS
		case RenderAPI::API::DX3D:				KE_CORE_ASSERT(false, "RendererAPI::DX3D -> Framebuffer Not Implemented!");
#endif // KE_PLATFORM_WINDOWS
		}

		KE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
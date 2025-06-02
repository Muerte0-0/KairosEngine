#include "kepch.h"
#include "GraphicsContext.h"

#include "Renderer.h"

#include "API/OpenGL/OpenGLContext.h"
#include "API/Vulkan/VulkanContext.h"

namespace Kairos
{
	GraphicsContext* GraphicsContext::Create(GLFWwindow* windowHandle)
	{
		switch (RenderAPI::GetAPI())
		{
			case RenderAPI::API::None:				KE_CORE_ASSERT(false, "RendererAPI::None -> Graphics Context Not Implemented!");
			case RenderAPI::API::OpenGL:			return new OpenGLContext(windowHandle);
			case RenderAPI::API::Vulkan:			return new VulkanContext(windowHandle);
			#ifdef KE_PLATFORM_WINDOWS
			case RenderAPI::API::DX3D:				KE_CORE_ASSERT(false, "RendererAPI::DX3D -> Graphics Context Not Implemented!");
			#endif // KE_PLATFORM_WINDOWS
		}

		KE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
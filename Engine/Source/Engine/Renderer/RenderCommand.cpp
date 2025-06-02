#include "kepch.h"
#include "RenderCommand.h"

#include "API/OpenGL/OpenGLRenderAPI.h"
#include "API/Vulkan/VulkanRenderAPI.h"

namespace Kairos
{
	RenderAPI* RenderCommand::CreateRenderAPI()
	{
		switch (RenderAPI::GetAPI())
		{
			case RenderAPI::API::None:				KE_CORE_ASSERT(false, "RendererAPI::None -> API Not Implemented!");
			case RenderAPI::API::OpenGL:			return new OpenGLRenderAPI();
			case RenderAPI::API::Vulkan:			return new VulkanRenderAPI();
			#ifdef KE_PLATFORM_WINDOWS
			case RenderAPI::API::DX3D:				KE_CORE_ASSERT(false, "RendererAPI::DX3D -> API Not Implemented!");
			#endif // KE_PLATFORM_WINDOWS
		}

		KE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	RenderAPI* RenderCommand::s_RendererAPI = RenderCommand::CreateRenderAPI();
}
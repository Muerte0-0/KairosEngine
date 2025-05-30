#include "kepch.h"
#include "ImGuiLayer.h"

#include "Engine/Renderer/RenderAPI.h"

#include "API/OpenGL/OpenGLImguiLayer.h"
#include "API/Vulkan/VulkanImGuiLayer.h"

namespace Kairos
{
    ImGuiLayer* Kairos::ImGuiLayer::Create()
    {
		switch (RenderAPI::GetAPI())
		{
			case RenderAPI::API::None:				KE_CORE_ASSERT(false, "RendererAPI::None -> ImGui Not Implemented!");
			case RenderAPI::API::OpenGL:			return new OpenGLImGuiLayer();
			case RenderAPI::API::Vulkan:			return new VulkanImGuiLayer();
			#ifdef KE_PLATFORM_WINDOWS
			case RenderAPI::API::DX3D:				KE_CORE_ASSERT(false, "RendererAPI::DX3D -> ImGui Not Implemented!");
			#endif // KE_PLATFORM_WINDOWS
		}

		KE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
    }
}

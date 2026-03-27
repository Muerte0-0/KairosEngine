#include "kepch.h"
#include "ImGuiLayer.h"

#include "APIs/Vulkan/VulkanImGuiLayer.h"

#ifdef KE_PLATFORM_WINDOWS
//#include "API/D3D11/D3D11ImGuiLayer.h"
//#include "API/D3D12/D3D12ImGuiLayer.h"
#endif // KE_PLATFORM_WINDOWS

namespace Engine
{
	Scope<ImGuiLayer> ImGuiLayer::Create()
	{
		return CreateScope<VulkanImGuiLayer>();
	}
}
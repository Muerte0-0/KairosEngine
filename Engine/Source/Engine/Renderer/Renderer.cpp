#include "kepch.h"
#include "Renderer.h"

#include "APIs/Vulkan/VulkanRenderAPI.h"

#ifdef PLATFORM_WINDOWS
#endif

namespace Engine
{
	Scope<Engine::RenderAPI> Engine::Renderer::s_API = nullptr;
	
	void Renderer::Init(API api, void* windowHandle)
	{
		switch (api)
		{
		case API::Vulkan:
			s_API = CreateScope<VulkanRenderAPI>();
			break;
#ifdef PLATFORM_WINDOWS
		case API::DX11:
			cerr << "API[Direct X 11]: Not Implemented" << "\n";
			break;
		case API::DX12:
			cerr << "API[Direct X 12]: Not Implemented" << "\n";
			break;
#endif
		default: 
			cerr << "API: Unknown API" << "\n";
			break;
		}

		s_API->Init(windowHandle);
	}

	void Renderer::BeginFrame()
	{
		s_API->BeginFrame();
	}

	void Renderer::EndFrame()
	{
		s_API->EndFrame();
	}

	void Renderer::Clear()
	{
		s_API->Clear();
	}
}

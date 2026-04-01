#include "kepch.h"
#include "Renderer.h"

#include "APIs/Vulkan/VulkanRenderAPI.h"

namespace Engine
{
	Scope<RenderAPI> Renderer::s_API = nullptr;

	void Renderer::Init(API api, void* windowHandle, const std::filesystem::path& shaderDirectory)
	{
		switch (api)
		{
		case API::Vulkan:
			s_API = CreateScope<VulkanRenderAPI>();
			break;
#if defined(PLATFORM_WINDOWS)
		case API::DX11:
			std::cerr << "API[Direct X 11]: Not Implemented\n";
			break;
		case API::DX12:
			std::cerr << "API[Direct X 12]: Not Implemented\n";
			break;
#endif
		default:
			std::cerr << "API: Unknown API\n";
			break;
		}

		s_API->Init(windowHandle, shaderDirectory);
	}

	void Renderer::BeginFrame()       { s_API->BeginFrame(); }
	void Renderer::DrawFrame()        { s_API->DrawFrame(); }
	void Renderer::EndFrame()         { s_API->EndFrame(); }
	void Renderer::WindowResized()    { s_API->WindowResized(); }

	Framebuffer* Renderer::GetFramebuffer()
	{
		return s_API->GetFramebuffer();
	}

	GraphicsPipeline* Renderer::GetGraphicsPipeline()
	{
		return s_API->GetGraphicsPipeline();
	}

	ShaderLibrary* Renderer::GetShaderLibrary()
	{
		return s_API->GetShaderLibrary();
	}

	void Renderer::ResizeFramebuffer(uint32_t width, uint32_t height)
	{
		s_API->ResizeFramebuffer(width, height);
	}
}

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
#ifdef PLATFORM_WINDOWS
		case API::DX11:
			ASSERT(false, "API[Direct X 11]: Not Implemented");
			break;
		case API::DX12:
			ASSERT(false, "API[Direct X 12]: Not Implemented");
			break;
#endif
		}

		s_API->Init(windowHandle, shaderDirectory);
	}

	void Renderer::BeginScene()       { s_API->BeginScene(); }
	void Renderer::DrawFrame()        { s_API->DrawFrame(); }
	void Renderer::EndScene()         { s_API->EndScene(); }
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

#include "kepch.h"
#include "Renderer.h"

#include "APIs/Vulkan/VulkanRenderAPI.h"
#include "RHI/Resources/Material.h"

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

	void Renderer::BeginScene()    { s_API->BeginScene(); }
	
	void Renderer::Shutdown()
	{
		Material::ResetDefault();
		s_API->ReleaseStaticResources();
		s_API->WaitIdle();
		s_API.reset();
	}
	
	void Renderer::DrawFrame()     { s_API->DrawFrame(); }
	void Renderer::EndScene()      { s_API->EndScene(); }
	void Renderer::WindowResized() { s_API->WindowResized(); }

	bool Renderer::IsFrameValid() { return s_API->IsFrameValid(); }

	ShaderLibrary* Renderer::GetShaderLibrary()
	{
		return s_API->GetShaderLibrary();
	}
}

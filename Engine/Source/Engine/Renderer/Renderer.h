#pragma once
#include "RHI/RenderAPI.h"
#include "RHI/Framebuffer.h"

namespace Engine
{
	class ShaderLibrary;

	class Renderer
	{
	public:
		static void Init(API api, void* windowHandle, const std::filesystem::path& shaderDirectory);
		
		static void BeginFrame();
		static void DrawFrame();
		static void EndFrame();
		
		static Framebuffer* GetFramebuffer();
		
		static void WindowResized();
		static void ResizeFramebuffer(uint32_t width, uint32_t height);
		
		static RenderAPI* GetAPI() { return s_API.get(); }

	private:
		static Scope<RenderAPI> s_API;
	};
}

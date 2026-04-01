#pragma once
#include "RHI/RenderAPI.h"
#include "RHI/Framebuffer.h"
#include "RHI/Shader.h"

namespace Engine
{
	class Renderer
	{
	public:
		static void Init(API api, void* windowHandle, const std::filesystem::path& shaderDirectory);

		static void BeginFrame();
		static void DrawFrame();
		static void EndFrame();

		static Framebuffer*     GetFramebuffer();
		static GraphicsPipeline* GetGraphicsPipeline();
		static ShaderLibrary*   GetShaderLibrary();

		static void WindowResized();
		static void ResizeFramebuffer(uint32_t width, uint32_t height);

		static RenderAPI* GetAPI() { return s_API.get(); }

	private:
		static Scope<RenderAPI> s_API;
	};
}

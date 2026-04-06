#pragma once
#include "RHI/RenderAPI.h"
#include "RHI/Shader.h"

namespace Engine
{
	class Renderer
	{
	public:
		static void Init(API api, void* windowHandle, const std::filesystem::path& shaderDirectory);

		static void BeginScene();
		static void DrawFrame();
		static void EndScene();

		static void WindowResized();

		static ShaderLibrary* GetShaderLibrary();

		static RenderAPI* GetAPI() { return s_API.get(); }

	private:
		static Scope<RenderAPI> s_API;
	};
}

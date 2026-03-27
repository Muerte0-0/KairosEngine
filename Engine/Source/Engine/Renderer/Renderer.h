#pragma once
#include "RHI/RenderAPI.h"

namespace Engine
{
	class Renderer
	{
	public:
		static void Init(API api, void* windowHandle);
		static void BeginFrame();
		static void DrawFrame();
		static void EndFrame();
		static void WindowResized();
		
		static RenderAPI* GetAPI() { return s_API.get(); }

	private:
		static Scope<RenderAPI> s_API;
	};
}

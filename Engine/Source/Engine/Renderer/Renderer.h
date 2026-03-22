#pragma once
#include "RHI/RenderAPI.h"

namespace Engine
{
	class Renderer
	{
	public:
		static void Init(API api, void* windowHandle);
		static void BeginFrame();
		static void EndFrame();
		static void Clear();

	private:
		static Scope<RenderAPI> s_API;
	};
}

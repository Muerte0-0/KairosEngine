#pragma once

enum class API
{
	None = 0,
	Vulkan,
	DX11,
	DX12
};

namespace Engine
{
	class RenderAPI
	{
	public:
		virtual ~RenderAPI() = default;

		virtual void Init(void* windowHandle) = 0;
		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void SetClearColor(float r, float g, float b, float a) = 0;
		virtual void Clear() = 0;
		
		virtual void WindowResized() = 0;
	};
}

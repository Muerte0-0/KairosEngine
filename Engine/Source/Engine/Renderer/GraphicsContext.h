#pragma once

struct GLFWwindow;

namespace Kairos
{
	class GraphicsContext
	{
	public:
		virtual void Init() = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual void Update() = 0;
		virtual void Cleanup() = 0;

		static GraphicsContext* Create(GLFWwindow* windowHandle);
	};
}
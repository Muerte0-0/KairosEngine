#pragma once
#include "Engine/Renderer/GraphicsContext.h"

#include <GLFW/glfw3.h>

namespace Kairos
{
	class VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SetVSync(bool enabled) override;
		virtual void SwapBuffers() override;
		virtual void Cleanup() override;

	private:
		GLFWwindow* m_WindowHandle;
	};
}
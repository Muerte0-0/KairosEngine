#pragma once
#include "Engine/Renderer/GraphicsContext.h"

#include <GLFW/glfw3.h>
#include <glad/gl.h>

namespace Kairos
{
	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SetVSync(bool enabled) override;
		virtual void Update() override;
		virtual void Cleanup() override;

	private:
		GLFWwindow* m_WindowHandle;
	};
}
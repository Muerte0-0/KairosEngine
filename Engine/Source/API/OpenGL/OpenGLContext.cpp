#include "kepch.h"
#include "OpenGLContext.h"

#include "glad/gl.h"

namespace Kairos
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle)
	{
		KE_CORE_ASSERT(windowHandle, "Window Handle is Null!");
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle);

		int status = gladLoadGL(glfwGetProcAddress);
		KE_CORE_ASSERT(status, "Failed to Initialize Glad!");

		KE_CORE_INFO("OpenGL Info");
		KE_CORE_INFO("	Version : {0}", (char*)glGetString(GL_VERSION));
		KE_CORE_INFO("  GLFW Version: {0}", (char*)glfwGetVersionString());
		KE_CORE_INFO("  Renderer: {0}", (char*)glGetString(GL_RENDERER));
		KE_CORE_INFO("Initialized OpenGL!");
	}

	void OpenGLContext::SetVSync(bool enabled)
	{
		glfwSwapInterval(enabled);
	}

	void OpenGLContext::Update()
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwSwapBuffers(m_WindowHandle);
	}

	void OpenGLContext::Cleanup()
	{
		glfwDestroyWindow(m_WindowHandle);
		glfwTerminate();
	}
}
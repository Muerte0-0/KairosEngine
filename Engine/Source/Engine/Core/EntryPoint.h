#pragma once
#include  "PlatformDetection.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

inline int Main(int argc, char** argv)
{
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	
	GLFWwindow* window = glfwCreateWindow(640, 480, "Hello World", nullptr, nullptr);
	
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

#ifdef PLATFORM_WINDOWS

inline bool g_ApplicationRunning = true;

#ifdef KE_DIST

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nShowCmd)
{
	return Main(__argc, __argv);
}

#else

int main(int argc, char** argv)
{
	return Main(argc, argv);
}

#endif

#endif

#ifdef PLATFORM_LINUX

int main(int argc, char** argv)
{
	return Main(argc, argv);
}

#endif

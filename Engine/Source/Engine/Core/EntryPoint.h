#pragma once
#include  "PlatformDetection.h"

#include <GLFW/glfw3.h>

#ifdef PLATFORM_WINDOWS

inline bool g_ApplicationRunning = true;

inline int Main(int argc, char** argv)
{
	GLFWwindow* window;
	
	if (!glfwInit())
		return -1;
	
	window = glfwCreateWindow(640, 480, "Hello World", nullptr, nullptr);
	
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	
	glfwMakeContextCurrent(window);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}
	
	glfwTerminate();
	return 0;
}

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
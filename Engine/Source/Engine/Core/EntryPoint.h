#pragma once
#include  "PlatformDetection.h"

inline bool g_ApplicationRunning = true;

inline int Main(int argc, char** argv)
{
	
	return 0;
}

#ifdef PLATFORM_WINDOWS

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

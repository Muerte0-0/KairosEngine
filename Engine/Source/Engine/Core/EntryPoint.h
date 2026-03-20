#pragma once

#ifdef PLATFORM_WINDOWS

inline bool g_ApplicationRunning = true;

int Main(int argc, char** argv)
{
	while (g_ApplicationRunning)
	{
		
	}
    
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
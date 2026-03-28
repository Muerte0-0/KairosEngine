#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/Application.h"

extern Engine::Application* Engine::CreateApplication(int argc, char** argv);

inline bool g_ApplicationRunning = true;

inline int Main(int argc, char** argv)
{
	while (g_ApplicationRunning)
	{
		Engine::Application* app = Engine::CreateApplication(argc, argv); // Create the Application Instance
		
		Engine::InitializeCore(app->GetApplicationSpecs()); // Initialize the Core systems of the Engine
        
		app->Initialize(); // Initialize Application
		app->Run(); // Run the Application Loop
		app->Shutdown(); // Shutdown Application
        
		delete app; // Clean up the Application instance
        
		Engine::ShutdownCore(); // Shutdown the Core systems of the Engine
	}
	
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

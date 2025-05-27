#pragma once
#include "Engine/Core/Base.h"
#include "Engine/Core/Application.h"

#ifdef KE_PLATFORM_WINDOWS

extern Kairos::Application* Kairos::CreateApplication();

int main(int argc, char** argv)
{
	Kairos::Log::Init();

	KE_PROFILE_BEGIN_SESSION("Startup", "KairosProfile-Startup.json");
	auto app = Kairos::CreateApplication();
	KE_PROFILE_END_SESSION();

	KE_PROFILE_BEGIN_SESSION("Runtime", "KairosProfile-Runtime.json");
	app->Run();
	KE_PROFILE_END_SESSION();

	KE_PROFILE_BEGIN_SESSION("Shutdown", "KairosProfile-Shutdown.json");
	delete app;
	KE_PROFILE_END_SESSION();
}

#endif
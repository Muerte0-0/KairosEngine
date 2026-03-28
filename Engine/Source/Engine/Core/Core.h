#pragma once
#include "Engine/Core/Application.h"
#include "Engine/Debugging/Log.h"

namespace Engine
{
	inline void InitializeCore(const ApplicationSpecification& applicationSpec)
	{
		Log::Init(applicationSpec.Name);
		
	}
	
	inline void ShutdownCore()
	{
		Log::Shutdown();
	}
}

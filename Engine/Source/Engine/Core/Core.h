#pragma once
#include "Engine/Debug/Log.h"

namespace Engine
{
	inline void InitializeCore()
	{
		Log::Init();
	}
	
	inline void ShutdownCore()
	{
		
	}
}

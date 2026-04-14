#pragma once

#include "Engine/Core/Base.h"
#include "Engine/Debugging/Log.h"

#ifdef KE_DEBUG

#define ASSERT(check, ...)\
{ \
	if (!(check)) \
	{ \
		LOG(::Engine::LogLevel::Error, "Assertion Failed: {0}", fmt::format(__VA_ARGS__)); \
		DEBUGBREAK(); \
	} \
}

#else

#define ASSERT(...)

#endif
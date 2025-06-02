#pragma once

#include "Engine/Core/PlatformDetection.h"

#include <memory>

#ifdef KE_DEBUG
#if defined(KE_PLATFORM_WINDOWS)
#define KE_DEBUGBREAK() __debugbreak()
#elif defined(KE_PLATFORM_LINUX)
#include <signal.h>
#define KE_DEBUGBREAK() raise(SIGTRAP)
#else
#error "Platform doesn't support debugbreak yet!"
#endif
#define KE_ENABLE_ASSERTS
#else
#define KE_DEBUGBREAK()
#endif

#define KE_EXPAND_MACRO(x) x
#define KE_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define KE_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace Kairos {

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}

#include "Engine/Core/Log.h"
#include "Engine/Core/Assert.h"
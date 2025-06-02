#pragma once

#include "Engine/Core/Base.h"
#include "Engine/Core/Log.h"
#include <filesystem>

#ifdef KE_ENABLE_ASSERTS

// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
#define KE_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { KE##type##ERROR(msg, __VA_ARGS__); KE_DEBUGBREAK(); } }
#define KE_INTERNAL_ASSERT_WITH_MSG(type, check, ...) KE_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define KE_INTERNAL_ASSERT_NO_MSG(type, check) KE_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", KE_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define KE_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define KE_INTERNAL_ASSERT_GET_MACRO(...) KE_EXPAND_MACRO( KE_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, KE_INTERNAL_ASSERT_WITH_MSG, KE_INTERNAL_ASSERT_NO_MSG) )

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define KE_ASSERT(...) KE_EXPAND_MACRO( KE_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
#define KE_CORE_ASSERT(...) KE_EXPAND_MACRO( KE_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
#define KE_ASSERT(...)
#define KE_CORE_ASSERT(...)
#endif
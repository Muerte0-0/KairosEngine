#pragma once

#include "Base.h"

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace Kairos
{
	class Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;

	};
}

// Core Log Macros
#define KE_CORE_TRACE(...)    ::Kairos::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define KE_CORE_INFO(...)     ::Kairos::Log::GetCoreLogger()->info(__VA_ARGS__)
#define KE_CORE_WARN(...)     ::Kairos::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define KE_CORE_ERROR(...)    ::Kairos::Log::GetCoreLogger()->error(__VA_ARGS__)
#define KE_CORE_FATAL(...)    ::Kairos::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// Client Log Macros
#define KE_TRACE(...)         ::Kairos::Log::GetClientLogger()->trace(__VA_ARGS__)
#define KE_INFO(...)          ::Kairos::Log::GetClientLogger()->info(__VA_ARGS__)
#define KE_WARN(...)          ::Kairos::Log::GetClientLogger()->warn(__VA_ARGS__)
#define KE_ERROR(...)         ::Kairos::Log::GetClientLogger()->error(__VA_ARGS__)
#define KE_FATAL(...)         ::Kairos::Log::GetClientLogger()->fatal(__VA_ARGS__)
#pragma once

#include "Base.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

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

template<typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector)
{
	return os << glm::to_string(vector);
}

template<typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix)
{
	return os << glm::to_string(matrix);
}

template<typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternion)
{
	return os << glm::to_string(quaternion);
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
#pragma once

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

#include <string>

namespace Engine
{
	enum class LogLevel
	{
		Trace,
		Info,
		Warning,
		Error,
		Critical
	};
	
	class Log
	{
	public:
		static void Init(const std::string& applicationName);
		static void Shutdown();

		inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }
	private:
		static std::shared_ptr<spdlog::logger> s_Logger;
	};
	
	template<typename... Args>
	static void LogMessage(spdlog::source_loc loc, LogLevel level, fmt::format_string<Args...> fmt, Args&&... args)
	{
		auto& logger = Log::GetLogger();

		std::string message = fmt::format("{}", fmt::format(fmt, std::forward<Args>(args)...));

		switch (level)
		{
		case LogLevel::Trace:    logger->log(loc, spdlog::level::trace, message); break;
		case LogLevel::Info:     logger->log(loc, spdlog::level::info, message); break;
		case LogLevel::Warning:  logger->log(loc, spdlog::level::warn, message); break;
		case LogLevel::Error:    logger->log(loc, spdlog::level::err, message); break;
		case LogLevel::Critical: logger->log(loc, spdlog::level::critical, message); break;
		}
	}
}

#define LOG(level, ...) ::Engine::LogMessage(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, level, __VA_ARGS__)

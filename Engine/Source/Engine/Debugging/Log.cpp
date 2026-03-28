#include "kepch.h"
#include "Log.h"

#include <filesystem>
#include <vector>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace Engine
{
	Ref<spdlog::logger> Log::s_Logger;

	namespace
	{
		std::filesystem::path GetExecutableDirectory()
		{
#ifdef PLATFORM_WINDOWS
			std::vector<char> buffer(MAX_PATH);

			while (true)
			{
				const DWORD copiedCharacters = GetModuleFileNameA(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
				if (copiedCharacters == 0)
					return std::filesystem::current_path();

				if (copiedCharacters < buffer.size() - 1)
					return std::filesystem::path(buffer.data()).parent_path();

				buffer.resize(buffer.size() * 2);
			}
#elif defined(PLATFORM_LINUX)
			std::error_code errorCode;
			const std::filesystem::path executablePath = std::filesystem::read_symlink("/proc/self/exe", errorCode);
			return errorCode ? std::filesystem::current_path() : executablePath.parent_path();
#else
			return std::filesystem::current_path();
#endif
		}
	}
	
	void Log::Init(const std::string& applicationName)
	{
		const std::filesystem::path logsDirectory = GetExecutableDirectory() / "Logs";
		std::filesystem::create_directories(logsDirectory);

		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(CreateRef<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(CreateRef<spdlog::sinks::basic_file_sink_mt>((logsDirectory / (applicationName + ".log")).string(), true));

		logSinks[0]->set_pattern("%^[%T] [%s: Line-%#] %v%$");
		logSinks[1]->set_pattern("[%T] [%l] [%s: Line-%#] %n: %v");
		//logSinks[2]->set_pattern("[%T] [%s: Line-%#] %n: %v"); // Imgui Logsink

		s_Logger = CreateRef<spdlog::logger>("Engine", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_Logger);
		s_Logger->set_level(spdlog::level::trace);
		s_Logger->flush_on(spdlog::level::trace);
	}

	void Log::Shutdown()
	{
		if (!s_Logger)
			return;

		spdlog::drop(s_Logger->name());
		s_Logger.reset();
	}
}

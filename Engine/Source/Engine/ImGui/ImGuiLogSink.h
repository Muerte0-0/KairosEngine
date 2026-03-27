#pragma once

#include <spdlog/sinks/base_sink.h>
#include <mutex>

namespace Engine
{
	class ImGuiLogSink : public spdlog::sinks::base_sink<std::mutex>
	{
	public:
		static std::shared_ptr<ImGuiLogSink> Get()
		{
			static auto instance = std::make_shared<ImGuiLogSink>();
			return instance;
		}

		void Draw(const char* title = "Console")
		{
			//m_LogWindow.Draw(title);
		}

		void Clear()
		{
			//m_LogWindow.Clear();
		}

	protected:
		void sink_it_(const spdlog::details::log_msg& msg) override
		{
			spdlog::memory_buf_t formatted;
			base_sink<std::mutex>::formatter_->format(msg, formatted);

			// Converting to string and adding it to ImGui log window
			std::string log_entry = fmt::to_string(formatted);
			//m_LogWindow.AddLog("%s", log_entry.c_str());
		}

		void flush_() override
		{
			
		}

	private:
		//LogWindow m_LogWindow;
	};
}
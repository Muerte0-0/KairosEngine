#pragma once

#include "Base.h"
#include "Timestep.h"
#include "Engine/Events/Event.h"

namespace Kairos
{
	struct WindowProps
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		WindowProps(const std::string& title = "Window Title", uint32_t width = 1280, uint32_t height = 720) : Title(title), Width(width), Height(height) {}
	};

	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;

		virtual void OnUpdate() = 0;

		virtual void SetTitle(std::string title) = 0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSyncEnabled() const = 0;

		virtual void* GetNativeWindow() const = 0;
		virtual void* GetGraphicsContext() const = 0;

		static Window* Create(const WindowProps& props = WindowProps());
	};

}
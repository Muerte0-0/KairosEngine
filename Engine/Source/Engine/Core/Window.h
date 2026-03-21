#pragma once

#include "Engine/Events/Event.h"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <string>
#include <functional>

namespace Engine
{

	struct WindowSpecification
	{
		std::string Title;
		uint32_t Width = 1280;
		uint32_t Height = 720;
		bool CustomTitleBar = false;
		bool IsResizeable = true;
		bool VSync = true;

		using EventCallbackFn = std::function<void(Event&)>;
		EventCallbackFn EventCallback;
	};

	class Window
	{
	public:
		Window(const WindowSpecification& specification);
		~Window();

		void Create();
		void Destroy();

		void Update();

		void RaiseEvent(Event& event);

		glm::vec2 GetFramebufferSize() const;
		glm::vec2 GetMousePos() const;

		bool ShouldClose() const;

		GLFWwindow* GetHandle() const { return m_Handle; }
	private:
		WindowSpecification m_Specification;

		GLFWwindow* m_Handle = nullptr;
	};
}
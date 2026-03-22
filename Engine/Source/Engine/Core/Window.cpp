#include "kepch.h"
#include "Window.h"

#include "Engine/Events/WindowEvents.h"
#include "Engine/Events/InputEvents.h"

#include <iostream>
#include <assert.h>

namespace Engine {

	Window::Window(const WindowSpecification& specification) : m_Specification(specification)
	{}

	Window::~Window()
	{
		Destroy();
	}

	void Window::Create()
	{
		glfwWindowHint(GLFW_RESIZABLE, m_Specification.IsResizeable ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		
		m_Handle = glfwCreateWindow(m_Specification.Width, m_Specification.Height,
			m_Specification.Title.c_str(), nullptr, nullptr);

		if (!m_Handle)
		{
			std::cerr << "Failed to create GLFW window!\n";
			assert(false);
		}

		glfwSetWindowUserPointer(m_Handle, this);
		
		glfwSetWindowCloseCallback(m_Handle, [](GLFWwindow* handle)
		{
			Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

			WindowClosedEvent event;
			window.RaiseEvent(event);
		});

		glfwSetWindowSizeCallback(m_Handle, [](GLFWwindow* handle, int width, int height)
		{
			Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));
			
			WindowResizeEvent event(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
			window.RaiseEvent(event);
		});

		glfwSetKeyCallback(m_Handle, [](GLFWwindow* handle, int key, int scancode, int action, int mods)
		{
			Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

			switch (action)
			{
				case GLFW_PRESS:
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, action == GLFW_REPEAT);
					window.RaiseEvent(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					window.RaiseEvent(event);
					break;
				}
			}
		});

		glfwSetMouseButtonCallback(m_Handle, [](GLFWwindow* handle, int button, int action, int mods)
		{
			Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					window.RaiseEvent(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					window.RaiseEvent(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_Handle, [](GLFWwindow* handle, double xOffset, double yOffset)
		{
			Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

			MouseScrolledEvent event(xOffset, yOffset);
			window.RaiseEvent(event);
		});

		glfwSetCursorPosCallback(m_Handle, [](GLFWwindow* handle, double x, double y)
		{
			Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

			MouseMovedEvent event(x, y);
			window.RaiseEvent(event);
		});
	}

	void Window::Destroy()
	{
		if (m_Handle)
			glfwDestroyWindow(m_Handle);

		m_Handle = nullptr;
	}

	void Window::Update()
	{
		
	}

	void Window::RaiseEvent(Event& event)
	{
		if (m_Specification.EventCallback)
			m_Specification.EventCallback(event);
	}

	glm::vec2 Window::GetFramebufferSize() const
	{
		int width, height;
		glfwGetFramebufferSize(m_Handle, &width, &height);
		return { width, height };
	}

	glm::vec2 Window::GetMousePos() const
	{
		double x, y;
		glfwGetCursorPos(m_Handle, &x, &y);
		return { static_cast<float>(x), static_cast<float>(y) };
	}

	bool Window::ShouldClose() const
	{
		return glfwWindowShouldClose(m_Handle) != 0;
	}

}
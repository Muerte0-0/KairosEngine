#include "kepch.h"
#include "Window.h"

#include "Engine/Utils/PlatformUtils.h"
#include "Engine/Events/WindowEvents.h"
#include "Engine/Events/InputEvents.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Engine
{
	namespace
	{
		/**
		 * @brief Resolve the path to the window icon.
		 *
		 * Walks up to the workspace root and appends the well-known
		 * relative path.  Returns an empty path when the workspace root
		 * cannot be found so the caller can skip icon loading gracefully.
		 */
		std::filesystem::path ResolveIconPath()
		{
			const auto root = PlatformUtils::ResolveWorkspaceRoot();
			if (!root)
			{
				LOG(LogLevel::Warning, "Window: could not resolve workspace root — skipping icon load.");
				return {};
			}

			return *root / "Engine" / "Resources" / "Icons" / "TempLogo.png";
		}
	}

	Window::Window(const WindowSpecification& specification)
		: m_Specification(specification)
	{}

	Window::~Window()
	{
		Destroy();
	}

	void Window::Create()
	{
		glfwWindowHint(GLFW_RESIZABLE,  m_Specification.IsResizeable    ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_MAXIMIZED,  m_Specification.LaunchMaximized ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		m_Handle = glfwCreateWindow(
			static_cast<int>(m_Specification.Width),
			static_cast<int>(m_Specification.Height),
			m_Specification.Title.c_str(),
			nullptr, nullptr);

		if (!m_Handle)
		{
			std::cerr << "Failed to create GLFW window!\n";
			assert(false);
		}

		glfwSetWindowUserPointer(m_Handle, this);

		// ---------------------------------------------------------------
		// Window icon
		// ---------------------------------------------------------------
		const std::filesystem::path iconPath = ResolveIconPath();
		if (!iconPath.empty())
		{
			GLFWimage icon{};
			int channels = 0;
			icon.pixels  = stbi_load(iconPath.string().c_str(),
									 &icon.width, &icon.height, &channels, 4);
			if (icon.pixels)
			{
				glfwSetWindowIcon(m_Handle, 1, &icon);
				stbi_image_free(icon.pixels);
			}
			else
			{
				LOG(LogLevel::Warning, "Window: failed to load icon from '{}'", iconPath.string());
			}
		}

		// ---------------------------------------------------------------
		// GLFW callbacks
		// ---------------------------------------------------------------
		glfwSetWindowCloseCallback(m_Handle, [](GLFWwindow* handle)
		{
			auto& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));
			WindowClosedEvent event;
			window.RaiseEvent(event);
		});

		glfwSetWindowSizeCallback(m_Handle, [](GLFWwindow* handle, int width, int height)
		{
			auto& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));
			WindowResizeEvent event(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
			window.RaiseEvent(event);
		});

		glfwSetKeyCallback(m_Handle, [](GLFWwindow* handle, int key, int /*scancode*/, int action, int /*mods*/)
		{
			auto& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));
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
			default: break;
			}
		});

		glfwSetMouseButtonCallback(m_Handle, [](GLFWwindow* handle, int button, int action, int /*mods*/)
		{
			auto& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));
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
			default: break;
			}
		});

		glfwSetScrollCallback(m_Handle, [](GLFWwindow* handle, double xOffset, double yOffset)
		{
			auto& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));
			MouseScrolledEvent event(xOffset, yOffset);
			window.RaiseEvent(event);
		});

		glfwSetCursorPosCallback(m_Handle, [](GLFWwindow* handle, double x, double y)
		{
			auto& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));
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

	void Window::Update() {}

	void Window::RaiseEvent(Event& event)
	{
		if (m_Specification.EventCallback)
			m_Specification.EventCallback(event);
	}

	glm::vec2 Window::GetFramebufferSize() const
	{
		int width, height;
		glfwGetFramebufferSize(m_Handle, &width, &height);
		return { static_cast<float>(width), static_cast<float>(height) };
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

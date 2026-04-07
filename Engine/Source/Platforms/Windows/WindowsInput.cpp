#include "kepch.h"

#ifdef PLATFORM_WINDOWS

#include "WindowsInput.h"

#include "Engine/Core/Application.h"
#include <GLFW/glfw3.h>

namespace Engine
{
	Input* Input::s_Instance = new WindowsInput();
	CursorMode Input::m_CursorMode = CursorMode::Normal;
	
	bool WindowsInput::IsKeyPressedImpl(KeyCode keycode)
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow()->GetHandle());
		auto state = glfwGetKey(window, static_cast<int32_t>(keycode));
		return state == GLFW_PRESS;
	}

	bool WindowsInput::IsMouseButtonPressedImpl(KeyCode button)
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow()->GetHandle());
		auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
		return state == GLFW_PRESS;
	}

	void WindowsInput::SetCursorLockModeImpl(CursorMode mode)
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow()->GetHandle());

		m_CursorMode = mode;

		switch (mode)
		{
		case CursorMode::Normal:
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			break;

		case CursorMode::Hidden:
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			break;

		case CursorMode::Locked:
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			break;
		}
	}

	glm::vec2 WindowsInput::GetMousePosImpl()
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow()->GetHandle());
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);

		return { static_cast<float>(xPos), static_cast<float>(yPos) };
	}

	glm::vec2 WindowsInput::GetMouseDeltaImpl()
	{
		return { 0, 0};
	}

	float WindowsInput::GetMouseXImpl()
	{
		return GetMousePos().x;
	}

	float WindowsInput::GetMouseYImpl()
	{
		return GetMousePos().y;
	}
}

#endif
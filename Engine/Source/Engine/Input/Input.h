#pragma once

#include "Engine/Input/KeyCodes.h"

#include <glm/glm.hpp>

namespace Engine
{
	class Input
	{
	public:
		inline static bool IsKeyPressed(KeyCode keycode) { return s_Instance->IsKeyPressedImpl(keycode); }

		inline static bool IsMouseButtonPressed(KeyCode button) { return s_Instance->IsMouseButtonPressedImpl(button); }
		inline static glm::vec2 GetMousePos() { return s_Instance->GetMousePosImpl(); }
		inline static glm::vec2 GetMouseDelta() { return s_Instance->GetMouseDeltaImpl(); }
		inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
		inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }

		inline static void SetCursorLockMode(CursorMode mode) { s_Instance->SetCursorLockModeImpl(mode); }
		inline static CursorMode GetCursorLockMode() { return m_CursorMode; }

	protected:
		virtual bool IsKeyPressedImpl(KeyCode keycode) = 0;

		virtual bool IsMouseButtonPressedImpl(KeyCode button) = 0;
		virtual void SetCursorLockModeImpl(CursorMode mode) = 0;
		virtual glm::vec2 GetMousePosImpl() = 0;
		virtual glm::vec2 GetMouseDeltaImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;

		static CursorMode m_CursorMode;

	private:
		static Input* s_Instance;
	};
}
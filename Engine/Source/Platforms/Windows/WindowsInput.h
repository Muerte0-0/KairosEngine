#pragma once

#ifdef PLATFORM_WINDOWS

#include "Engine/Input/Input.h"

namespace Engine
{
	class WindowsInput : public Input
	{
	protected:
		virtual bool IsKeyPressedImpl(KeyCode keycode) override;

		virtual bool IsMouseButtonPressedImpl(KeyCode button) override;
		virtual void SetCursorLockModeImpl(CursorMode mode) override;
		virtual glm::vec2 GetMousePosImpl() override;
		virtual glm::vec2 GetMouseDeltaImpl() override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;
	};
}

#endif
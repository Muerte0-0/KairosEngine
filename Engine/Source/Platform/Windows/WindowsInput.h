#pragma once

#include "Engine/Input/Input.h"

namespace Kairos
{
	class WindowsInput : public Input
	{
	protected:
		virtual bool IsKeyPressedImpl(KeyCode keycode) override;

		virtual bool IsMouseButtonPressedImpl(KeyCode button) override;
		virtual std::pair<float, float> GetMousePosImpl();
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;
	};
}
#pragma once

#include "Engine/Core/Layer.h"

#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"

namespace Kairos
{
	class ImGuiLayer : public Layer
	{
	public:
		~ImGuiLayer() = default;

		virtual void Begin() = 0;
		virtual void End() = 0;

		static ImGuiLayer* Create();
	private:
		float m_Time = 0.0f;
	};
}
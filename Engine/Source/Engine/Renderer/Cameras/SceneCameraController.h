#pragma once
#include "SceneCamera.h"

#include "Engine/Events/Event.h"
#include "Engine/Events/InputEvents.h"
#include "Engine/Input/Input.h"

#include <glm/glm.hpp>

namespace Engine
{
	enum class SceneCameraMode : uint8_t
	{
		None = 0,
		FreeFly,    // RMB held → WASD + mouse look
		Orbit,      // Alt + LMB → orbit focal point
		Pan,        // Alt + MMB → pan
	};
	
	class SceneCameraController
	{
	public:
		explicit SceneCameraController(SceneCamera& camera);
		
		void OnUpdate(float deltaTime);
		void OnEvent(Event& event);

		// Let the editor tell the controller if it should consume input.
		void SetViewportFocused(bool focused) { m_ViewportFocused = focused; }
		void SetViewportHovered(bool hovered) { m_ViewportHovered = hovered; }

		SceneCameraMode GetMode() const { return m_Mode; }

	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);

		SceneCamera&		m_Camera;
		SceneCameraMode		m_Mode				= SceneCameraMode::None;
		
		glm::vec2			m_LastMousePos		{ 0.0f };
		
		bool				m_ViewportFocused	= false;
		bool				m_ViewportHovered	= false;
	};
}

#include "kepch.h"
#include "SceneCameraController.h"

namespace Engine
{
	SceneCameraController::SceneCameraController(SceneCamera& camera) : m_Camera(camera)
	{
	}

	void SceneCameraController::OnUpdate(float deltaTime)
	{
		glm::vec2 mousePos = Input::GetMousePos();

		// A mode is active → keep running regardless of focus/hover flags
		// (cursor lock makes those unreliable mid-flight).
		// No active mode → only run when hovered (hover is enough to enter any mode;
		// focus is only needed for keyboard and is set reactively by the viewport).
		bool modeActive = (m_Mode != SceneCameraMode::None);
		if (!modeActive && !m_ViewportHovered)
		{
			m_LastMousePos = mousePos;
			return;
		}

        bool alt = Input::IsKeyPressed(KeyBoard::LeftAlt);

        // --------------------------------------------------------
        // Mode Detection
        // --------------------------------------------------------

        SceneCameraMode newMode;
        if (alt && Input::IsMouseButtonPressed(Mouse::ButtonLeft))
            newMode = SceneCameraMode::Orbit;
        else if (alt && Input::IsMouseButtonPressed(Mouse::ButtonMiddle))
            newMode = SceneCameraMode::Pan;
        else if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
            newMode = SceneCameraMode::FreeFly;
        else
            newMode = SceneCameraMode::None;

        // Seed m_LastMousePos on the frame a mode first becomes active so the
        // very first delta is always zero — prevents camera snapping.
        if (newMode != SceneCameraMode::None && m_Mode == SceneCameraMode::None)
            m_LastMousePos = mousePos;

        m_Mode = newMode;

        // Compute delta AFTER seeding.
        glm::vec2 delta = mousePos - m_LastMousePos;
        m_LastMousePos = mousePos;

        // --------------------------------------------------------
        // Mode Execution
        // --------------------------------------------------------

        switch (m_Mode)
        {
        case SceneCameraMode::FreeFly:
        	{
        		// Only rotate if there is meaningful mouse movement.
        		if (glm::length(delta) > 0.0f)
        			m_Camera.FreeFly_Rotate(delta);

        		// Movement — WASD only makes sense when viewport has keyboard focus.
        		if (m_ViewportFocused)
        		{
        			glm::vec3 direction(0.0f);
        			if (Input::IsKeyPressed(KeyBoard::W)) direction.z += 1.0f;
        			if (Input::IsKeyPressed(KeyBoard::S)) direction.z -= 1.0f;
        			if (Input::IsKeyPressed(KeyBoard::D)) direction.x += 1.0f;
        			if (Input::IsKeyPressed(KeyBoard::A)) direction.x -= 1.0f;
        			if (Input::IsKeyPressed(KeyBoard::E)) direction.y += 1.0f;
        			if (Input::IsKeyPressed(KeyBoard::Q)) direction.y -= 1.0f;
        			m_Camera.FreeFly_Move(direction, deltaTime);
        		}
        		break;
        	}

        case SceneCameraMode::Orbit:
        	{
        		if (glm::length(delta) > 0.0f)
        			m_Camera.Orbit(delta);
        		break;
        	}

        case SceneCameraMode::Pan:
        	{
        		if (glm::length(delta) > 0.0f)
        			m_Camera.Pan(delta);
        		break;
        	}

        case SceneCameraMode::None:
        	break;
        }
	}

	void SceneCameraController::OnEvent(Event& event)
	{
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(&SceneCameraController::OnMouseScrolled));
	}

	bool SceneCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		// Accept scroll whenever the viewport is hovered or a mode is active.
		if (!m_ViewportHovered && m_Mode == SceneCameraMode::None)
			return false;

		m_Camera.Zoom(static_cast<float>(e.GetYOffset()));
		return true;
	}
}

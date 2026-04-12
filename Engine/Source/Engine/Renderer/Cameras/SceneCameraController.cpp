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

		// While a mode is active the cursor is locked — ImGui's hovered/focused
		// flags become unreliable (cursor sits at lock pos, may read as outside
		// viewport). Keep running so the camera doesn't freeze mid-flight.
		// Only bail out when no mode is active AND focus/hover are lost.
		bool modeActive = (m_Mode != SceneCameraMode::None);
		if (!modeActive && (!m_ViewportFocused || !m_ViewportHovered))
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
        // very first delta is always zero — prevents camera snapping when
        // re-entering the viewport after clicking elsewhere.
        if (newMode != SceneCameraMode::None && m_Mode == SceneCameraMode::None)
            m_LastMousePos = mousePos;

        m_Mode = newMode;

        // Compute delta AFTER seeding so the first frame of any mode is zero.
        glm::vec2 delta = mousePos - m_LastMousePos;
        m_LastMousePos = mousePos;

        // --------------------------------------------------------
        // Mode Execution
        // --------------------------------------------------------

        switch (m_Mode)
        {
        case SceneCameraMode::FreeFly:
        	{        		
        		// Rotate camera
        		m_Camera.FreeFly_Rotate(delta);

        		// Movement (WASD + QE)
        		glm::vec3 direction(0.0f);

        		if (Input::IsKeyPressed(KeyBoard::W)) direction.z += 1.0f;
        		if (Input::IsKeyPressed(KeyBoard::S)) direction.z -= 1.0f;
        		if (Input::IsKeyPressed(KeyBoard::D)) direction.x += 1.0f;
        		if (Input::IsKeyPressed(KeyBoard::A)) direction.x -= 1.0f;
        		if (Input::IsKeyPressed(KeyBoard::E)) direction.y += 1.0f;
        		if (Input::IsKeyPressed(KeyBoard::Q)) direction.y -= 1.0f;

        		m_Camera.FreeFly_Move(direction, deltaTime);
        		break;
        	}

        case SceneCameraMode::Orbit:
        	{
        		m_Camera.Orbit(delta);
        		break;
        	}

        case SceneCameraMode::Pan:
        	{
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

        dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN(&SceneCameraController::OnMouseMoved));
	}

	bool SceneCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		if (!m_ViewportHovered)
			return false;

		m_Camera.Zoom(static_cast<float>(e.GetYOffset()));
		return true;
	}

	bool SceneCameraController::OnMouseMoved(MouseMovedEvent& e)
	{
		return false;
	}
}

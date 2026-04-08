#include "kepch.h"
#include "SceneCameraController.h"

namespace Engine
{
	SceneCameraController::SceneCameraController(SceneCamera& camera) : m_Camera(camera)
	{
	}

	void SceneCameraController::OnUpdate(float deltaTime)
	{
		if (!m_ViewportFocused)
			return;

        glm::vec2 mousePos = Input::GetMousePos();
		
		if (m_FirstFrame)
		{
			m_LastMousePos = mousePos;
			m_FirstFrame = false;
		}
		
        glm::vec2 delta = mousePos - m_LastMousePos;
        m_LastMousePos = mousePos;

        bool alt = Input::IsKeyPressed(KeyBoard::LeftAlt);

        // --------------------------------------------------------
        // Mode Detection
        // --------------------------------------------------------

        if (alt && Input::IsMouseButtonPressed(Mouse::ButtonLeft))
            m_Mode = SceneCameraMode::Orbit;
        else if (alt && Input::IsMouseButtonPressed(Mouse::ButtonMiddle))
            m_Mode = SceneCameraMode::Pan;
        else if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
            m_Mode = SceneCameraMode::FreeFly;
        else
            m_Mode = SceneCameraMode::None;

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
        		if (Input::IsKeyPressed(KeyBoard::E)) direction.y -= 1.0f;
        		if (Input::IsKeyPressed(KeyBoard::Q)) direction.y += 1.0f;

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

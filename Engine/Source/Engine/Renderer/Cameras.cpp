#include "kepch.h"
#include "Cameras.h"

#include "Engine/Input/Input.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Kairos
{
	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top) : m_ProjectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)), m_ViewMatrix(1.0f)
	{
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::RecalculateViewMatrix()
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));

		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::OnUpdate(Timestep deltaTime)
	{
		if (Input::IsKeyPressed(KeyBoard::W))
			m_Position.y += m_CameraMoveSpeed;
		else if (Input::IsKeyPressed(KeyBoard::S))
			m_Position.y -= m_CameraMoveSpeed;

		if (Input::IsKeyPressed(KeyBoard::A))
			m_Position.x -= m_CameraMoveSpeed;
		else if (Input::IsKeyPressed(KeyBoard::D))
			m_Position.x += m_CameraMoveSpeed;

		if (Input::IsKeyPressed(KeyBoard::Q))
			m_Rotation += m_CameraRotationSpeed;
		else if (Input::IsKeyPressed(KeyBoard::E))
			m_Rotation -= m_CameraRotationSpeed;

		RecalculateViewMatrix();
	}
}
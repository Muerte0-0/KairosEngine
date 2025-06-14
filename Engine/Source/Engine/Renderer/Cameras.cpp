#include "kepch.h"
#include "Cameras.h"

#include "Engine/Input/Input.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace Kairos
{
	// Orthographic Camera

	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
		: m_ProjectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)), m_ViewMatrix(1.0f)
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
			m_Position.y += m_CameraMoveSpeed * deltaTime;
		else if (Input::IsKeyPressed(KeyBoard::S))
			m_Position.y -= m_CameraMoveSpeed * deltaTime;

		if (Input::IsKeyPressed(KeyBoard::A))
			m_Position.x -= m_CameraMoveSpeed * deltaTime;
		else if (Input::IsKeyPressed(KeyBoard::D))
			m_Position.x += m_CameraMoveSpeed * deltaTime;

		if (Input::IsKeyPressed(KeyBoard::Q))
			m_Rotation += m_CameraRotationSpeed * deltaTime;
		else if (Input::IsKeyPressed(KeyBoard::E))
			m_Rotation -= m_CameraRotationSpeed * deltaTime;

		RecalculateViewMatrix();
	}

	// Perspective Camera

	PerspectiveCamera::PerspectiveCamera(float fov, float viewportWidth, float viewportHeight, float nearPlane, float farPlane)
		: m_ProjectionMatrix(glm::perspectiveFov(fov, viewportWidth, viewportHeight, nearPlane, farPlane)), m_FOV(fov), m_NearPlane(nearPlane), m_FarPlane(farPlane)
	{
		RecalculateViewMatrix();
	}

	void PerspectiveCamera::ViewportResized(float viewportWidth, float viewportHeight)
	{
		m_ProjectionMatrix = glm::perspectiveFov(m_FOV, viewportWidth, viewportHeight, m_NearPlane, m_FarPlane);

		RecalculateViewMatrix();
	}

	void PerspectiveCamera::OnUpdate(Timestep deltaTime)
	{
		if (Input::IsKeyPressed(KeyBoard::W))
			m_Position.z -= m_CameraMoveSpeed * deltaTime;
		else if (Input::IsKeyPressed(KeyBoard::S))
			m_Position.z += m_CameraMoveSpeed * deltaTime;

		if (Input::IsKeyPressed(KeyBoard::A))
			m_Position.x -= m_CameraMoveSpeed * deltaTime;
		else if (Input::IsKeyPressed(KeyBoard::D))
			m_Position.x += m_CameraMoveSpeed * deltaTime;

		if (Input::IsKeyPressed(KeyBoard::Q))
			m_Position.y -= m_CameraMoveSpeed * deltaTime;
		else if (Input::IsKeyPressed(KeyBoard::E))
			m_Position.y += m_CameraMoveSpeed * deltaTime;

		if (Input::IsKeyPressed(KeyBoard::Left))
			m_Rotation.y -= m_CameraMoveSpeed * deltaTime;
		else if (Input::IsKeyPressed(KeyBoard::Right))
			m_Rotation.y += m_CameraMoveSpeed * deltaTime;

		if (Input::IsKeyPressed(KeyBoard::Up))
			m_Rotation.x -= m_CameraMoveSpeed * deltaTime;
		else if (Input::IsKeyPressed(KeyBoard::Down))
			m_Rotation.x += m_CameraMoveSpeed * deltaTime;

		RecalculateViewMatrix();
	}

	void PerspectiveCamera::RecalculateViewMatrix()
	{
		m_Transform = glm::translate(glm::mat4(1.0f), m_Position) 
			* glm::eulerAngleXYZ(m_Rotation.x, m_Rotation.y, m_Rotation.z);

		m_ViewMatrix = glm::inverse(m_Transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
}
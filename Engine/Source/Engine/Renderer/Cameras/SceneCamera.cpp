#include "kepch.h"
#include "SceneCamera.h"

#include "glm/gtc/matrix_transform.hpp"

namespace Engine
{
	SceneCamera::SceneCamera(const SceneCameraProps& props) : m_Props(props)
	{
		RecalculateView();
	}

	void SceneCamera::FreeFly_Rotate(glm::vec2 delta)
	{
		m_Yaw   += delta.x * m_Props.RotateSpeed;
		m_Pitch += delta.y * m_Props.RotateSpeed;

		// Prevent upside-down chaos
		m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);

		RecalculateView();
	}

	void SceneCamera::FreeFly_Move(glm::vec3 localDirection, float deltaTime)
	{
		glm::vec3 velocity = localDirection * m_Props.MoveSpeed * deltaTime;

		m_Position += GetForward() * velocity.z;
		m_Position += GetRight()   * velocity.x;
		m_Position += GetUp()      * velocity.y;

		// Keep focal point in sync (so orbit feels natural after flying)
		m_FocalPoint = m_Position + GetForward();

		RecalculateView();
	}

	void SceneCamera::Orbit(glm::vec2 delta)
	{
		glm::vec3 direction = m_Position - m_FocalPoint;
		float distance = glm::length(direction);

		m_Yaw   += delta.x * m_Props.OrbitSpeed;
		m_Pitch += delta.y * m_Props.OrbitSpeed;
		m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);

		// Recalculate direction from yaw/pitch
		glm::vec3 newDir = {
			cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch)),
			sin(glm::radians(m_Pitch)),
			sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch))
		};

		m_Position = m_FocalPoint - newDir * distance;

		RecalculateView();
	}

	void SceneCamera::Pan(glm::vec2 delta)
	{
		float distance = glm::length(m_Position - m_FocalPoint);

		float panSpeed = m_Props.PanSpeed * distance;

		glm::vec3 right = GetRight();
		glm::vec3 up    = GetUp();

		glm::vec3 offset = (-right * delta.x + up * delta.y) * panSpeed;

		m_Position   += offset;
		m_FocalPoint += offset;

		RecalculateView();
	}

	void SceneCamera::Zoom(float scrollDelta)
	{
		glm::vec3 direction = m_FocalPoint - m_Position;
		float distance = glm::length(direction);

		float zoomAmount = scrollDelta * m_Props.ZoomSpeed * distance;

		// Prevent flipping through focal point
		if (distance - zoomAmount < 0.1f)
			zoomAmount = distance - 0.1f;

		m_Position += glm::normalize(direction) * zoomAmount;

		RecalculateView();
	}

	void SceneCamera::SetFocalPoint(const glm::vec3& point)
	{
		m_FocalPoint = point;
		RecalculateView();
	}

	void SceneCamera::SetPosition(const glm::vec3& position)
	{
		m_Position = position;
		RecalculateView();
	}

	void SceneCamera::OnViewportResize(uint32_t width, uint32_t height)
	{
		if (height == 0 || width == 0) return;
		
		SetAspectRatio(static_cast<float>(width) / static_cast<float>(height));
	}

	void SceneCamera::RecalculateView()
	{
		glm::vec3 forward = {
			cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch)),
			sin(glm::radians(m_Pitch)),
			sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch))
		};

		glm::vec3 target = m_Position + glm::normalize(forward);

		m_View = glm::lookAt(
			m_Position,
			target,
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
	}

	glm::vec3 SceneCamera::GetForward() const
	{
		return glm::normalize(glm::vec3{
			cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch)),
			sin(glm::radians(m_Pitch)),
			sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch))
		});
	}

	glm::vec3 SceneCamera::GetRight() const
	{
		return glm::normalize(glm::cross(GetForward(), glm::vec3(0.0f, 1.0f, 0.0f)));
	}

	glm::vec3 SceneCamera::GetUp() const
	{
		return glm::normalize(glm::cross(GetRight(), GetForward()));
	}
}

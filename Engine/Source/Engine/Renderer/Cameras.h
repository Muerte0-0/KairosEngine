#pragma once

#include "Engine/Core/Timestep.h"

#include <glm/glm.hpp>

namespace Kairos
{
	class OrthographicCamera
	{
	public:
		OrthographicCamera(float left, float right, float bottom, float top);

		const glm::vec3& GetPosition() { return m_Position; }
		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }

		const float& GetRotation() { return m_Rotation; }
		void SetRotation(const float& rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }

		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

		void OnUpdate(Timestep delaTime);

	private:
		void RecalculateViewMatrix();

	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position = {0.0f, 0.0f , 0.0f};
		float m_Rotation = 0.0f;

		float m_CameraMoveSpeed = 1.0f;
		float m_CameraRotationSpeed = 1.0f;
	};

	class PerspectiveCamera
	{
	public:
		PerspectiveCamera(float fov, float viewportWidth, float viewportHeight, float nearPlane = 0.1f, float farPlane = 1000.0f);

		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
		const glm::mat4& GetTransform() const { return m_Transform; }

		void OnUpdate(Timestep delaTime);

		void ViewportResized(float viewportWidth, float viewportHeight);

	private:
		void RecalculateViewMatrix();

	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position = { 0.0f, 0.0f , 4.0f };
		glm::vec3 m_Rotation = { 0.0f, 0.0f , 0.0f };

		glm::mat4 m_Transform;

		float m_CameraMoveSpeed = 4.0f;
		float m_CameraRotationSpeed = 1.0f;

		float m_FOV = 45.0f;
		float m_NearPlane = 0.1f;
		float m_FarPlane = 1000.0f;
	};
}
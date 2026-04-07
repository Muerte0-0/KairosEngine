#pragma once
#include "ProjectionCamera.h"
#include <glm/glm.hpp>

namespace Engine
{
	struct SceneCameraProps
	{
		float MoveSpeed    = 5.0f;
		float RotateSpeed  = 0.1f;
		float OrbitSpeed   = 0.5f;
		float PanSpeed     = 0.01f;
		float ZoomSpeed    = 0.25f;
	};
	
	class SceneCamera : public ProjectionCamera
	{
	public:
		explicit SceneCamera(const SceneCameraProps& props = {});
		
		// Free-fly: delta is screen-space mouse delta in pixels.
		void FreeFly_Rotate(glm::vec2 delta);
		void FreeFly_Move(glm::vec3 localDirection, float deltaTime);

		// Orbit around focal point.
		void Orbit(glm::vec2 delta);
		void Pan(glm::vec2 delta);
		void Zoom(float scrollDelta);

		void SetFocalPoint(const glm::vec3& point);
		glm::vec3 GetFocalPoint() const { return m_FocalPoint; }

		void SetPosition(const glm::vec3& position);
		glm::vec3 GetPosition() const { return m_Position; }

		// Called when viewport resizes.
		void OnViewportResize(uint32_t width, uint32_t height);

		const SceneCameraProps& GetProps() const { return m_Props; }

	private:
		void RecalculateView();

		glm::vec3 GetForward() const;
		glm::vec3 GetRight()   const;
		glm::vec3 GetUp()      const;

		SceneCameraProps m_Props;

		glm::vec3 m_Position   { 0.0f, 0.0f, 4.0f };
		glm::vec3 m_FocalPoint { 0.0f };

		float m_Yaw   = -90.0f;
		float m_Pitch = 0.0f;
	};
}

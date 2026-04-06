#pragma once
#include "Camera.h"

namespace Engine
{
	struct PerspectiveProps
	{
		float FOV         = 45.0f;
		float AspectRatio = 16.0f / 9.0f;
		float Near        = 0.1f;
		float Far         = 1000.0f;
	};

	struct OrthographicProps
	{
		float Size        = 10.0f;   // half-height in world units
		float AspectRatio = 16.0f / 9.0f;
		float Near        = -1.0f;
		float Far         = 1.0f;
	};
	
	class ProjectionCamera : public Camera
	{
	public:
		explicit ProjectionCamera(const PerspectiveProps& props = {});

		// Switching modes rebuilds the projection matrix immediately.
		void SetPerspective(const PerspectiveProps& props);
		void SetOrthographic(const OrthographicProps& props);

		// Call when the viewport size changes.
		void SetAspectRatio(float aspectRatio);

		// Fine-tuning without switching modes.
		void SetFOV(float fovDegrees);
		void SetNearFar(float nearPlane, float farPlane);

		ProjectionType GetProjectionType() const { return m_ProjectionType; }
		float GetFOV()         const { return m_PerspProps.FOV; }
		float GetAspectRatio() const { return m_PerspProps.AspectRatio; }
		float GetNear()        const { return m_PerspProps.Near; }
		float GetFar()         const { return m_PerspProps.Far; }

	private:
		void RecalculateProjection();

		ProjectionType    m_ProjectionType;
		PerspectiveProps  m_PerspProps;
		OrthographicProps m_OrthoProps;
	};
}

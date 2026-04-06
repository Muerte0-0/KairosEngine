#include "kepch.h"
#include "ProjectionCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Engine
{
	ProjectionCamera::ProjectionCamera(const PerspectiveProps& props) : m_ProjectionType(ProjectionType::Perspective), m_PerspProps(props)
	{
		RecalculateProjection();
	}

	void ProjectionCamera::SetPerspective(const PerspectiveProps& props)
	{
		m_ProjectionType = ProjectionType::Perspective;
		m_PerspProps = props;
		
		RecalculateProjection();
	}

	void ProjectionCamera::SetOrthographic(const OrthographicProps& props)
	{
		m_ProjectionType = ProjectionType::Orthographic;
		m_OrthoProps = props;
		
		RecalculateProjection();
	}

	void ProjectionCamera::SetAspectRatio(float aspectRatio)
	{
		if (m_ProjectionType == ProjectionType::Perspective)
			m_PerspProps.AspectRatio = aspectRatio;
		else
			m_OrthoProps.AspectRatio = aspectRatio;

		RecalculateProjection();
	}

	void ProjectionCamera::SetFOV(float fovDegrees)
	{
		m_PerspProps.FOV = fovDegrees;
		
		if (m_ProjectionType == ProjectionType::Perspective)
			RecalculateProjection();
	}

	void ProjectionCamera::SetNearFar(float nearPlane, float farPlane)
	{
		if (m_ProjectionType == ProjectionType::Perspective)
		{
			m_PerspProps.Near = nearPlane;
			m_PerspProps.Far  = farPlane;
		}
		else
		{
			m_OrthoProps.Near = nearPlane;
			m_OrthoProps.Far  = farPlane;
		}

		RecalculateProjection();
	}

	void ProjectionCamera::RecalculateProjection()
	{
		if (m_ProjectionType == ProjectionType::Perspective)
		{
			const float fovRad = glm::radians(m_PerspProps.FOV);

			m_Projection = glm::perspective(
				fovRad,
				m_PerspProps.AspectRatio,
				m_PerspProps.Near,
				m_PerspProps.Far
			);
		}
		else
		{
			float orthoHeight = m_OrthoProps.Size;
			float orthoWidth  = orthoHeight * m_OrthoProps.AspectRatio;

			float left   = -orthoWidth;
			float right  =  orthoWidth;
			float bottom = -orthoHeight;
			float top    =  orthoHeight;

			m_Projection = glm::ortho(
				left, right,
				bottom, top,
				m_OrthoProps.Near,
				m_OrthoProps.Far
			);
		}
		
		m_Projection[1][1] *= -1.0f;
	}
}

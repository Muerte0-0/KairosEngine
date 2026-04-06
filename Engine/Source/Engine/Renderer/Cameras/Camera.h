#pragma once

#include <glm/glm.hpp>

namespace Engine
{
	enum class ProjectionType : uint8_t { Perspective = 0, Orthographic };
	
	class Camera
	{
	public:
		Camera() = default;
		explicit Camera(const glm::mat4& projection) : m_Projection(projection) {}
		Camera(const glm::mat4& view, const glm::mat4& projection) : m_View(view), m_Projection(projection) {}

		const glm::mat4& GetView()           const { return m_View; }
		const glm::mat4& GetProjection()     const { return m_Projection; }
		glm::mat4        GetViewProjection() const { return m_Projection * m_View; }

		void SetView(const glm::mat4& view)               { m_View = view; }
		void SetProjection(const glm::mat4& projection)   { m_Projection = projection; }

	protected:
		glm::mat4 m_View       { 1.0f };
		glm::mat4 m_Projection { 1.0f };
	};
}

#pragma once
#include "ProjectionCamera.h"
#include <entt.hpp>
#include <glm/glm.hpp>

namespace Engine
{
	// GameCamera: runtime camera attached to an ECS entity.
	// Inherits projection from ProjectionCamera; computes view from entity transform.
	// Does NOT hold a raw pointer to the registry — caller owns sync.
	class GameCamera : public ProjectionCamera
	{
	public:
		GameCamera() = default;
		explicit GameCamera(const PerspectiveProps& props) : ProjectionCamera(props) {}

		// ------------------------------------------------------------------
		// Entity binding
		// ------------------------------------------------------------------

		// Bind this camera to an entity. After binding, call SyncFromEntityTransform
		// each frame with the entity's world transform to update the view matrix.
		void BindToEntity(entt::entity id)   { m_BoundEntity = id; m_Bound = true; }
		void UnbindEntity()                  { m_Bound = false; m_BoundEntity = entt::null; }
		bool IsBound()             const     { return m_Bound; }
		entt::entity GetBoundEntity() const  { return m_BoundEntity; }

		// ------------------------------------------------------------------
		// View matrix sync
		// ------------------------------------------------------------------

		// Called by the scene each frame when in Play mode (or by editor preview).
		// worldTransform is the entity's final world-space TRS matrix.
		void SyncFromEntityTransform(const glm::mat4& worldTransform);

		// Fallback: set view directly when not bound to an entity.
		void SetViewFromTransform(const glm::vec3& position, const glm::vec3& eulerRadians);

	private:
		bool         m_Bound       = false;
		entt::entity m_BoundEntity = entt::null;
	};
}

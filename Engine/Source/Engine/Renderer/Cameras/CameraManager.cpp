#include "kepch.h"
#include "CameraManager.h"
#include "Engine/Scene/Components.h"

namespace Engine
{
	void CameraManager::UpdateFromRegistry(entt::registry& registry)
	{
		m_ActiveGameCamera = nullptr;

		// Find first CameraComponent marked Primary, sync its view from world transform.
		auto view = registry.view<CameraComponent, TransformComponent>();
		for (auto [entity, cam, tc] : view.each())
		{
			if (!cam.Primary)
				continue;

			// Sync view matrix from the entity's world transform.
			if (cam.Camera.IsBound())
				cam.Camera.SyncFromEntityTransform(tc.WorldTransform);
			else
				cam.Camera.SyncFromEntityTransform(tc.GetTransform());

			m_ActiveGameCamera = &cam.Camera;
			break; // only one primary
		}
	}

	const Camera* CameraManager::GetActiveCamera() const
	{
		if (m_Mode == CameraManagerMode::Play && m_ActiveGameCamera)
			return m_ActiveGameCamera;
		
		return m_SceneCamera;
	}
}

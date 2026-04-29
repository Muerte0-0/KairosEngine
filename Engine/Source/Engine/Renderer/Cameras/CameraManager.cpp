#include "kepch.h"
#include "CameraManager.h"

#include "Engine/Core/Application.h"
#include "Engine/Scene/Components.h"

namespace Engine
{
	void CameraManager::UpdateFromRegistry(entt::registry& registry)
	{
		m_ActiveGameCamera = nullptr;

		GameCamera* firstFound = nullptr;

		auto view = registry.view<CameraComponent, TransformComponent>();
		for (auto [entity, cam, tc] : view.each())
		{
			if (!firstFound)
				firstFound = &cam.Camera;

			if (!cam.Primary)
				continue;

			if (cam.Camera.IsBound())
				cam.Camera.SyncFromEntityTransform(tc.WorldTransform);
			else
				cam.Camera.SyncFromEntityTransform(tc.GetTransform());

			m_ActiveGameCamera = &cam.Camera;
			break;
		}

		// Fallback to first available camera if no primary is set.
		if (!m_ActiveGameCamera && firstFound)
			m_ActiveGameCamera = firstFound;
	}

	const Camera* CameraManager::GetActiveCamera() const
	{
		if ((Application::Get().GetEngineMode() == EngineMode::Play || Application::Get().GetEngineMode() == EngineMode::Paused) && m_ActiveGameCamera)
			return m_ActiveGameCamera;

		return m_SceneCamera;
	}
}

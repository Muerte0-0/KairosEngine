#pragma once
#include "SceneCamera.h"
#include "GameCamera.h"
#include "Engine/Core/EngineState.h"
#include <entt.hpp>

namespace Engine
{
	class CameraManager
	{
	public:
		CameraManager() = default;

		void SetSceneCamera(SceneCamera* camera) { m_SceneCamera = camera; }

		// Scans registry for the primary Camera Component + syncs its view from transform.
		// Call once per frame.
		void UpdateFromRegistry(entt::registry& registry);

		// Returns SceneCamera in Editor mode, primary GameCamera in Play/Paused mode.
		[[nodiscard]] const Camera* GetActiveCamera() const;

		[[nodiscard]] bool HasActiveCamera() const { return GetActiveCamera() != nullptr; }

		// Direct access for editor preview / ray-cast (always the editor camera).
		[[nodiscard]] const SceneCamera* GetSceneCamera() const { return m_SceneCamera; }

	private:
		SceneCamera* m_SceneCamera      = nullptr;
		GameCamera*  m_ActiveGameCamera = nullptr;  // non-owning; points into CameraComponent
	};
}

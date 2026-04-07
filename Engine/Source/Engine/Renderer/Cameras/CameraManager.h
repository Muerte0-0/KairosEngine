#pragma once
#include "SceneCamera.h"
#include <entt.hpp>

namespace Engine
{
	enum class CameraManagerMode : uint8_t
	{
		Editor,   // SceneCamera is active
		Play,     // Primary GameCamera (from ECS) is active
	};
	
	class CameraManager
	{
	public:
		CameraManager() = default;

		void SetSceneCamera(SceneCamera* camera) { m_SceneCamera = camera; }

		// Call every frame in Play mode to sync the primary game camera.
		void UpdateFromRegistry(entt::registry& registry);

		void SetMode(CameraManagerMode mode) { m_Mode = mode; }
		CameraManagerMode GetMode() const    { return m_Mode; }

		[[nodiscard]] const Camera* GetActiveCamera() const { return m_SceneCamera; }

		[[nodiscard]] bool HasActiveCamera() const { return GetActiveCamera() != nullptr; }

	private:
		CameraManagerMode	m_Mode				= CameraManagerMode::Editor;
		SceneCamera*		m_SceneCamera		= nullptr;
		const Camera*		m_ActiveGameCamera	= nullptr;
	
	};
}

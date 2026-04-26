#pragma once
#include "Engine.h"
#include "Panel.h"
#include <functional>
#include <filesystem>

namespace Kairos
{
	class ViewportPanel : public Panel
	{
	public:
		void Init(SceneRenderer* renderer, SceneCamera* camera,
		          SceneCameraController* controller, CameraManager* cameraManager,
		          SceneHierarchyPanel* hierarchy);

		void OnImGuiRender() override;

		bool IsHovered() const { return m_Hovered; }
		bool IsFocused() const { return m_Focused; }

		int  GetGizmoType()              const { return m_GizmoType; }
		void SetGizmoType(int type)            { m_GizmoType = type; }

		int  GetGizmoMode()              const { return m_GizmoMode; }
		void SetGizmoMode(int mode)            { m_GizmoMode = mode; }

		const glm::vec2& GetSize()       const { return m_Size; }
		const glm::vec2* GetBounds()     const { return m_Bounds; }

		// Drag-drop callbacks — set by EditorLayer
		std::function<void(const std::filesystem::path&)> OnSceneDrop;
		std::function<void(const std::filesystem::path&)> OnMeshDrop;

	private:
		void DrawGizmos();

		SceneRenderer*         m_Renderer        = nullptr;
		SceneCamera*           m_Camera          = nullptr;
		SceneCameraController* m_Controller      = nullptr;
		CameraManager*         m_CameraManager   = nullptr;
		SceneHierarchyPanel*   m_Hierarchy        = nullptr;

		bool      m_Hovered   = false;
		bool      m_Focused   = false;
		glm::vec2 m_Size      = { 1280.f, 720.f };
		glm::vec2 m_Bounds[2] = {};
		int       m_GizmoType = -1;
		int       m_GizmoMode = 0; // 0 = LOCAL, 1 = WORLD
	};
}

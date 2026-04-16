#pragma once
#include "Engine.h"

// Panels
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"

namespace Kairos
{
	class EditorLayer : public Engine::Layer
	{
		public
		:
		EditorLayer() = default;
		virtual 
		~EditorLayer()
		override = default;

		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate(float DeltaTime) override;
		void OnFixedUpdate(float DeltaTime) override;

		void OnRender() override;
		void OnImGuiRender() override;

		void OnEvent(Engine::Event& event) override;
	private:
		bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = {1280, 720};
		glm::vec2 m_ViewportBounds[2];

		Ref<Scene> m_ActiveScene;
		std::filesystem::path m_ActiveScenePath;
		Scope<SceneRenderer> m_SceneRenderer;
		Scope<SceneCamera> m_SceneCamera;
		Scope<SceneCameraController> m_SceneCameraController;
		CameraManager m_CameraManager;

		Entity m_CubeEntity;
		Entity m_CubeEntity2;
		Entity m_TestModelEntity;
		
		int m_GizmoType = -1;
		
		// ----------- Panels ----------- //
		
		Scope<SceneHierarchyPanel> m_SceneHierarchyPanel;
		Scope<ContentBrowserPanel> m_ContentBrowserPanel;
		
		// ------------------------------ //
		
		// ----------- ImGui ----------- //
		void DrawMenuBar();
		void DrawViewport();

		// Debug
		void DrawImGuiDebug();

		bool m_ShowConsole = true;
		void DrawConsole();
		// ----------------------------- //
		
		bool OnKeyPressedEvent(KeyPressedEvent& event);
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& event);

		// Returns the entity under the viewport mouse cursor, or an invalid Entity.
		// Cycles through overlapping hits if current selection is the front-most.
		Entity PickEntityAtMouse();

		void NewProject();
		void OpenProject(const std::filesystem::path& path);
		void SaveProject();
		
		void OpenScene(const std::filesystem::path& filepath);
		void SaveScene();
		void SaveSceneAs();
	};
}

#include "EditorLayer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>

#include "Panels/SceneHierarchyPanel.h"

#include "Engine/Scene/SceneSerializer.h"

#include "ImGuizmo.h"

namespace
{
	Ref<Mesh> CreateDefaultCubeMesh()
	{
		// Per-face normals for a unit cube.
		// Vertex layout: Position, Normal, Tangent, Bitangent, TexCoord
		// matching Kairos::Vertex / GetVertexLayout() exactly.
		std::vector<Vertex> vertices = {
			// +Z face (front) — normal  0, 0, 1
			{ { -0.5f, -0.5f,  0.5f }, {  0.f,  0.f,  1.f }, {  1.f,  0.f,  0.f }, {  0.f,  1.f,  0.f }, { 0.f, 0.f } },
			{ {  0.5f, -0.5f,  0.5f }, {  0.f,  0.f,  1.f }, {  1.f,  0.f,  0.f }, {  0.f,  1.f,  0.f }, { 1.f, 0.f } },
			{ {  0.5f,  0.5f,  0.5f }, {  0.f,  0.f,  1.f }, {  1.f,  0.f,  0.f }, {  0.f,  1.f,  0.f }, { 1.f, 1.f } },
			{ { -0.5f,  0.5f,  0.5f }, {  0.f,  0.f,  1.f }, {  1.f,  0.f,  0.f }, {  0.f,  1.f,  0.f }, { 0.f, 1.f } },

			// -Z face (back) — normal  0, 0,-1
			{ {  0.5f, -0.5f, -0.5f }, {  0.f,  0.f, -1.f }, { -1.f,  0.f,  0.f }, {  0.f,  1.f,  0.f }, { 0.f, 0.f } },
			{ { -0.5f, -0.5f, -0.5f }, {  0.f,  0.f, -1.f }, { -1.f,  0.f,  0.f }, {  0.f,  1.f,  0.f }, { 1.f, 0.f } },
			{ { -0.5f,  0.5f, -0.5f }, {  0.f,  0.f, -1.f }, { -1.f,  0.f,  0.f }, {  0.f,  1.f,  0.f }, { 1.f, 1.f } },
			{ {  0.5f,  0.5f, -0.5f }, {  0.f,  0.f, -1.f }, { -1.f,  0.f,  0.f }, {  0.f,  1.f,  0.f }, { 0.f, 1.f } },

			// -X face (left) — normal -1, 0, 0
			{ { -0.5f, -0.5f, -0.5f }, { -1.f,  0.f,  0.f }, {  0.f,  0.f,  1.f }, {  0.f,  1.f,  0.f }, { 0.f, 0.f } },
			{ { -0.5f, -0.5f,  0.5f }, { -1.f,  0.f,  0.f }, {  0.f,  0.f,  1.f }, {  0.f,  1.f,  0.f }, { 1.f, 0.f } },
			{ { -0.5f,  0.5f,  0.5f }, { -1.f,  0.f,  0.f }, {  0.f,  0.f,  1.f }, {  0.f,  1.f,  0.f }, { 1.f, 1.f } },
			{ { -0.5f,  0.5f, -0.5f }, { -1.f,  0.f,  0.f }, {  0.f,  0.f,  1.f }, {  0.f,  1.f,  0.f }, { 0.f, 1.f } },

			// +X face (right) — normal  1, 0, 0
			{ {  0.5f, -0.5f,  0.5f }, {  1.f,  0.f,  0.f }, {  0.f,  0.f, -1.f }, {  0.f,  1.f,  0.f }, { 0.f, 0.f } },
			{ {  0.5f, -0.5f, -0.5f }, {  1.f,  0.f,  0.f }, {  0.f,  0.f, -1.f }, {  0.f,  1.f,  0.f }, { 1.f, 0.f } },
			{ {  0.5f,  0.5f, -0.5f }, {  1.f,  0.f,  0.f }, {  0.f,  0.f, -1.f }, {  0.f,  1.f,  0.f }, { 1.f, 1.f } },
			{ {  0.5f,  0.5f,  0.5f }, {  1.f,  0.f,  0.f }, {  0.f,  0.f, -1.f }, {  0.f,  1.f,  0.f }, { 0.f, 1.f } },

			// +Y face (top) — normal  0, 1, 0
			{ { -0.5f,  0.5f,  0.5f }, {  0.f,  1.f,  0.f }, {  1.f,  0.f,  0.f }, {  0.f,  0.f, -1.f }, { 0.f, 0.f } },
			{ {  0.5f,  0.5f,  0.5f }, {  0.f,  1.f,  0.f }, {  1.f,  0.f,  0.f }, {  0.f,  0.f, -1.f }, { 1.f, 0.f } },
			{ {  0.5f,  0.5f, -0.5f }, {  0.f,  1.f,  0.f }, {  1.f,  0.f,  0.f }, {  0.f,  0.f, -1.f }, { 1.f, 1.f } },
			{ { -0.5f,  0.5f, -0.5f }, {  0.f,  1.f,  0.f }, {  1.f,  0.f,  0.f }, {  0.f,  0.f, -1.f }, { 0.f, 1.f } },

			// -Y face (bottom) — normal  0,-1, 0
			{ { -0.5f, -0.5f, -0.5f }, {  0.f, -1.f,  0.f }, {  1.f,  0.f,  0.f }, {  0.f,  0.f,  1.f }, { 0.f, 0.f } },
			{ {  0.5f, -0.5f, -0.5f }, {  0.f, -1.f,  0.f }, {  1.f,  0.f,  0.f }, {  0.f,  0.f,  1.f }, { 1.f, 0.f } },
			{ {  0.5f, -0.5f,  0.5f }, {  0.f, -1.f,  0.f }, {  1.f,  0.f,  0.f }, {  0.f,  0.f,  1.f }, { 1.f, 1.f } },
			{ { -0.5f, -0.5f,  0.5f }, {  0.f, -1.f,  0.f }, {  1.f,  0.f,  0.f }, {  0.f,  0.f,  1.f }, { 0.f, 1.f } },
		};

		std::vector<uint32_t> indices = {
			 0,  1,  2,   2,  3,  0,   // +Z
			 4,  5,  6,   6,  7,  4,   // -Z
			 8,  9, 10,  10, 11,  8,   // -X
			12, 13, 14,  14, 15, 12,   // +X
			16, 17, 18,  18, 19, 16,   // +Y
			20, 21, 22,  22, 23, 20,   // -Y
		};

		return Mesh::Create(std::move(vertices), std::move(indices));
	}
}

namespace Kairos
{
	void EditorLayer::OnAttach()
	{
		Layer::OnAttach();

		m_ActiveScene = CreateRef<Scene>();
		m_SceneRenderer = CreateScope<SceneRenderer>();
		m_SceneCamera = CreateScope<SceneCamera>();
		m_SceneCameraController = CreateScope<SceneCameraController>(*m_SceneCamera);
		m_CameraManager.SetSceneCamera(m_SceneCamera.get());
		m_CameraManager.SetMode(CameraManagerMode::Editor);

		m_CubeEntity = m_ActiveScene->CreateEntity("Cube");
		m_CubeEntity.AddComponent<MeshComponent>().Mesh = CreateDefaultCubeMesh();
		m_CubeEntity.GetComponent<TransformComponent>().Translation = glm::vec3(-2.0f, 0.0f, 0.0f);
		
		m_CubeEntity2 = m_ActiveScene->CreateEntity("Cube 2");
		m_CubeEntity2.AddComponent<MeshComponent>().Mesh = CreateDefaultCubeMesh();
		m_CubeEntity2.GetComponent<TransformComponent>().Translation = glm::vec3(2.0f, 0.0f, 0.0f);
		
		// Load a model from disk
		Model loadedModel = ModelFactory::Load("D:/Dev/Projects/KairosEngine/Editor/Resources/Models/Prop_Crate.fbx");

		if (loadedModel.MeshData)
		{
			m_TestModelEntity = m_ActiveScene->CreateEntity(loadedModel.Name);
			m_TestModelEntity.AddComponent<MeshComponent>().Mesh = loadedModel.MeshData;
			m_TestModelEntity.GetComponent<TransformComponent>().Translation = glm::vec3(0.0f, 0.0f, 0.0f);
		}
		else
		{
			LOG(LogLevel::Error, "Failed to load model!");
		}
		
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OnDetach()
	{
		Layer::OnDetach();
	}

	void EditorLayer::OnUpdate(float DeltaTime)
	{
		Layer::OnUpdate(DeltaTime);
		
		m_SceneCameraController->SetViewportFocused(m_ViewportFocused);
		m_SceneCameraController->SetViewportHovered(m_ViewportHovered);
		
		m_SceneCameraController->OnUpdate(DeltaTime);
	
		if (m_SceneCameraController->GetMode() != SceneCameraMode::None)
			Input::SetCursorLockMode(CursorMode::Locked);
		else
			Input::SetCursorLockMode(CursorMode::Normal);
	}

	void EditorLayer::OnFixedUpdate(float DeltaTime)
	{
		Layer::OnFixedUpdate(DeltaTime);
	}

	void EditorLayer::OnRender()
	{
		Layer::OnRender();

		if (!m_SceneRenderer)
			return;

		const float width = (std::max)(m_ViewportSize.x, 1.0f);
		const float height = (std::max)(m_ViewportSize.y, 1.0f);
		
		m_SceneRenderer->BeginScene(m_CameraManager);
		m_ActiveScene->OnRender(*m_SceneRenderer);
		m_SceneRenderer->EndScene();
	}

	void EditorLayer::OnImGuiRender()
	{
		static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;

		ImGuiWindowFlags dockspaceWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

		dockspaceWindowFlags |= ImGuiWindowFlags_NoTitleBar;
		dockspaceWindowFlags |= ImGuiWindowFlags_NoCollapse;
		dockspaceWindowFlags |= ImGuiWindowFlags_NoResize;
		dockspaceWindowFlags |= ImGuiWindowFlags_NoMove;
		dockspaceWindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
		dockspaceWindowFlags |= ImGuiWindowFlags_NoNavFocus;
		dockspaceWindowFlags |= ImGuiWindowFlags_NoBackground;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("Editor Dockspace", nullptr, dockspaceWindowFlags);

		ImGui::PopStyleVar();
		ImGui::PopStyleVar(2);

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspaceID = ImGui::GetID("Editor Dockspace");
			ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);
		}

		DrawMenuBar();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		DrawViewport();

		ImGui::PopStyleVar();

		m_SceneHierarchyPanel.OnImGuiRender();

		ImGui::Begin("Content Browser");
		ImGui::End();

		if (m_ShowConsole)
			DrawConsole();

		ImGui::End();
	}

	void EditorLayer::OnEvent(Engine::Event& event)
	{
		Layer::OnEvent(event);
	
		m_SceneCameraController->OnEvent(event);
		
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(&EditorLayer::OnKeyPressedEvent));
	}

	void EditorLayer::DrawMenuBar()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Project", "Ctrl+N"))
				{
				}

				if (ImGui::MenuItem("Open Project", "Ctrl+O"))
				{
				}

				if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
				{
					SaveScene();
				}
				
				if (ImGui::MenuItem("Save All", "Ctrl+Shift+S"))
				{
				}
				
				if (ImGui::MenuItem("Exit"))
				{
					Engine::Application::Get().Stop();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Tools"))
			{
				if (ImGui::MenuItem("New Script"))
				{
				}

				if (ImGui::MenuItem("Asset Browser"))
				{
				}

				if (ImGui::MenuItem("Console", nullptr, &m_ShowConsole))
				{
					LOG(Engine::LogLevel::Info, "Console {}", m_ShowConsole ? "Shown" : "Hidden");
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("About"))
			{
				if (ImGui::MenuItem("About Kairos Engine"))
				{
				}

				if (ImGui::MenuItem("Documentation"))
				{
				}

				if (ImGui::MenuItem("GitHub Repository"))
				{
					string url = "https://github.com/Muerte0-0/KairosEngine";

#ifdef PLATFORM_WINDOWS
					system(("start " + url).c_str());
#else
#if PLATFORM_LINUX
					system(("xdg-open " + url).c_str());
#endif
#endif
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	}

	void EditorLayer::DrawViewport()
	{
		ImGui::Begin("Viewport");
		
		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = glm::vec2(viewportPanelSize.x, viewportPanelSize.y);

		Engine::Framebuffer* framebuffer = m_SceneRenderer ? m_SceneRenderer->GetFramebuffer() : nullptr;
		if (framebuffer != nullptr && viewportPanelSize.x > 0.0f && viewportPanelSize.y > 0.0f)
		{
			m_SceneRenderer->Resize(
				static_cast<uint32_t>(viewportPanelSize.x),
				static_cast<uint32_t>(viewportPanelSize.y));
    	
			m_SceneCamera->OnViewportResize(
				static_cast<uint32_t>(viewportPanelSize.x),
				 static_cast<uint32_t>(viewportPanelSize.y));

			if (void* textureID = framebuffer->GetImGuiTextureID())
			{
				ImGui::Image(textureID, viewportPanelSize, ImVec2(0, 1), ImVec2(1, 0));
			}
		}
		else
		{
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Not Implemented Yet! :)");
		}

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && m_ViewportHovered && !m_ViewportFocused)
		{
			ImGui::SetWindowFocus();
			m_ViewportFocused = true;
		}
		
		// Gizmos
		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		
		if (selectedEntity && m_GizmoType != -1)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
			
			// Entity Transform
			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();
			
			// Snapping
			bool snap = Input::IsKeyPressed(KeyBoard::LeftControl);
			float snapValue = 0.5f;
			// Snap to 5 Degrees for Rotation
			if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 5.0f;
			
			float snapValues[3] = { snapValue, snapValue, snapValue };
			
			ImGuizmo::Manipulate(glm::value_ptr(m_CameraManager.GetActiveCamera()->GetView()),
				glm::value_ptr(m_CameraManager.GetActiveCamera()->GetProjection()), 
				static_cast<ImGuizmo::OPERATION>(m_GizmoType), ImGuizmo::LOCAL, glm::value_ptr(transform), nullptr,
				snap ? snapValues : nullptr);
			
			if (ImGuizmo::IsUsing())
			{
				glm::vec3 translation, rotation, scale;
				Math::DecomposeTransform(transform, translation, rotation, scale);
				
				glm::vec3 deltaRotation = rotation - tc.Rotation;
				
				tc.Translation = translation;
				tc.Rotation += deltaRotation;
				tc.Scale = scale;
			}
		}
		
		ImGui::End();
	}

	void EditorLayer::DrawImGuiDebug()
	{
		ImGuiIO& io = ImGui::GetIO();

		ImGui::Begin("Debug Info");

		ImGui::Text("ConfigFlags:");
		ImGui::Text("  ViewportsEnable: %d", (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0);
		ImGui::Text("  DockingEnable: %d", (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) != 0);

		ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
		ImGui::Text("Viewports: %d", platform_io.Viewports.Size);

		for (int i = 0; i < platform_io.Viewports.Size; i++)
		{
			ImGuiViewport* vp = platform_io.Viewports[i];
			ImGui::Text("  Viewport %d: Pos(%.0f, %.0f) Size(%.0f, %.0f) DrawData: %s",
						i, vp->Pos.x, vp->Pos.y, vp->Size.x, vp->Size.y,
						vp->DrawData ? "Yes" : "No");
		}
		ImGui::End();
	}

	void EditorLayer::DrawConsole()
	{
	}

	bool EditorLayer::OnKeyPressedEvent(KeyPressedEvent& event)
	{
		if (event.IsRepeat())
			return false;
		
		if (m_SceneCameraController->GetMode() != SceneCameraMode::None)
			return false;
		
		bool control = Input::IsKeyPressed(KeyBoard::LeftControl) || Input::IsKeyPressed(KeyBoard::RightControl);
		bool shift = Input::IsKeyPressed(KeyBoard::LeftShift) || Input::IsKeyPressed(KeyBoard::RightShift);

		switch (event.GetKeyCode())
		{
		case KeyBoard::S:
			if (control)
			{
				if (shift)
					SaveSceneAs();
				else
					SaveScene();
			}
			break;
		case KeyBoard::Q:
			m_GizmoType = -1;
			break;
		case KeyBoard::W:
			m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			break;
		case KeyBoard::E:
			m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			break;
		case KeyBoard::R:
			m_GizmoType = ImGuizmo::OPERATION::SCALE;
			break;
		default: break;
		}
		
		return true;
	}

	void EditorLayer::SaveScene() const
	{
		SceneSerializer serializer(m_ActiveScene);
		serializer.Serialize("Content/Scenes/Example.kairos");
		
		LOG(LogLevel::Info, "Saving Scene");
	}

	void EditorLayer::SaveSceneAs()
	{
		// To-DO
	}
}

#include "EditorLayer.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Panels/SceneHierarchyPanel.h"

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
		
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

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
	}

	void EditorLayer::OnDetach()
	{
		Layer::OnDetach();
	}

	void EditorLayer::OnUpdate(float DeltaTime)
	{
		Layer::OnUpdate(DeltaTime);
		
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

				if (ImGui::MenuItem("Save Project", "Ctrl+S"))
				{
				}

				if (ImGui::MenuItem("Save Project As", "Ctrl+Shift+S"))
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
		
		if (m_ViewportHovered && !m_ViewportFocused)
		{
			ImGui::SetWindowFocus();
			m_ViewportFocused = true;
		}
		
		if (!m_ViewportHovered && m_ViewportFocused)
		{
			ImGui::SetWindowFocus(nullptr);
			m_ViewportFocused = false;
		}
		
		m_SceneCameraController->SetViewportFocused(m_ViewportFocused);
		m_SceneCameraController->SetViewportHovered(m_ViewportHovered);

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
}
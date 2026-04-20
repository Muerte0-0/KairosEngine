#include "EditorLayer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>

#include "imgui_internal.h"
#include "Panels/SceneHierarchyPanel.h"

#include "Engine/Scene/SceneSerializer.h"

#include "ImGuizmo.h"

constexpr const char* KPROJ_FILTER = "Kairos Project\0*.kproj\0\0";

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
		
		OpenProject();
		
		m_SceneHierarchyPanel->SetContext(m_ActiveScene);

		m_ViewportPanel->Init(m_SceneRenderer.get(), m_SceneCamera.get(),
		                      m_SceneCameraController.get(), &m_CameraManager,
		                      m_SceneHierarchyPanel.get());

		m_ViewportPanel->OnSceneDrop = [this](const std::filesystem::path& path)
		{
			OpenScene(path);
		};

		m_ViewportPanel->OnMeshDrop = [this](const std::filesystem::path& path)
		{
			auto* editorAM = static_cast<EditorAssetManager*>(
				Project::GetActive()->GetAssetManager().get());

			AssetHandle handle = editorAM->ImportAsset(path);
			if (static_cast<uint64_t>(handle) == NullAssetHandle)
			{
				LOG(LogLevel::Warning, "Mesh drag-drop: import failed for '{}'.", path.string());
				return;
			}

			Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(handle);
			if (!mesh)
			{
				LOG(LogLevel::Warning, "Mesh drag-drop: asset loaded but Mesh cast failed for '{}'.", path.string());
				return;
			}

			std::string name  = path.stem().string();
			Entity entity     = m_ActiveScene->CreateEntity(name);
			auto& mc          = entity.AddComponent<MeshComponent>();
			mc.SetMeshAsset(handle, mesh, mesh->GetMaterials());
			m_SceneHierarchyPanel->SetSelectedEntity(entity);
			LOG(LogLevel::Info, "Spawned mesh entity '{}' from drag-drop.", name);
		};
	}

	void EditorLayer::OnDetach()
	{
		Layer::OnDetach();
	}

	void EditorLayer::OnUpdate(float DeltaTime)
	{
		Layer::OnUpdate(DeltaTime);
		
		m_SceneCameraController->SetViewportFocused(m_ViewportPanel->IsFocused());
		m_SceneCameraController->SetViewportHovered(m_ViewportPanel->IsHovered());
		
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

		m_SceneRenderer->BeginScene(m_CameraManager);
		m_ActiveScene->OnRender(*m_SceneRenderer);
		m_SceneRenderer->EndScene();
	}

	void EditorLayer::OnImGuiRender()
	{
		SetupOuterDockspace();
		DrawLevelEditorWindow();

		// Tool windows (Material Editor, Texture Editor, etc.)
		for (auto it = m_OpenWindows.begin(); it != m_OpenWindows.end();)
		{
			(*it)->OnImGuiRender();
			if (!(*it)->IsOpen())
				it = m_OpenWindows.erase(it);
			else
				++it;
		}
	}

	void EditorLayer::OnEvent(Engine::Event& event)
	{
		Layer::OnEvent(event);
	
		m_SceneCameraController->OnEvent(event);
		
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(&EditorLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(&EditorLayer::OnMouseButtonPressedEvent));
	}

	void EditorLayer::SetupOuterDockspace()
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoDocking   | ImGuiWindowFlags_NoTitleBar    |
			ImGuiWindowFlags_NoCollapse  | ImGuiWindowFlags_NoResize      |
			ImGuiWindowFlags_NoMove      | ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus  | ImGuiWindowFlags_NoBackground  |
			ImGuiWindowFlags_MenuBar;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("##OuterWindow", nullptr, flags);
		ImGui::PopStyleVar(3);

		m_OuterDockID = ImGui::GetID("##OuterDockspace");
		ImGui::DockSpace(m_OuterDockID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

		DrawMenuBar();

		ImGui::End();
	}

	void EditorLayer::DrawLevelEditorWindow()
	{
		// Fixed title — MUST NOT change, title change = new ImGui window = loses dock
		// Scene name displayed inside the window instead
		ImGuiCond dockCond = m_LevelEditorLayoutBuilt ? ImGuiCond_FirstUseEver : ImGuiCond_Always;
		ImGui::SetNextWindowDockID(m_OuterDockID, dockCond);
		ImGui::Begin("Level Editor", nullptr, ImGuiWindowFlags_NoCollapse);

		// Show scene name as text inside the window
		std::string sceneName = m_ActiveScenePath.empty()
			? "Untitled"
			: m_ActiveScenePath.stem().string();
		ImGui::TextDisabled("Scene: %s", sceneName.c_str());
		ImGui::Separator();

		ImGuiID innerID = ImGui::GetID("##LevelEditorInner");
		ImGui::DockSpace(innerID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

		if (!m_LevelEditorLayoutBuilt)
		{
			BuildLevelEditorLayout(innerID);
			m_LevelEditorLayoutBuilt = true;
		}

		ImGui::End();

		// Panels dock into inner slots — window names must match DockBuilderDockWindow strings
		m_SceneHierarchyPanel->OnImGuiRender();   // "Scene Hierarchy"

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		m_ViewportPanel->OnImGuiRender();          // "Viewport"
		ImGui::PopStyleVar();

		m_PropertiesPanel->SetSelectedEntity(m_SceneHierarchyPanel->GetSelectedEntity());
		m_PropertiesPanel->OnImGuiRender();        // "Properties"

		m_ContentBrowserPanel->OnImGuiRender();    // "Content Browser"

		if (m_ShowConsole)
			DrawConsole();
	}

	void EditorLayer::BuildLevelEditorLayout(ImGuiID innerID)
	{
		ImGui::DockBuilderRemoveNode(innerID);
		ImGui::DockBuilderAddNode(innerID, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(innerID, ImGui::GetMainViewport()->WorkSize);

		// Split bottom first (full width) then work on the top half
		ImGuiID top, bottom;
		ImGui::DockBuilderSplitNode(innerID, ImGuiDir_Down, 0.45f, &bottom, &top);

		ImGuiID left, center, right;
		ImGui::DockBuilderSplitNode(top,    ImGuiDir_Left,  0.18f, &left,   &center);
		ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.25f, &right,  &center);

		ImGui::DockBuilderDockWindow("Scene Hierarchy", left);
		ImGui::DockBuilderDockWindow("Viewport",        center);
		ImGui::DockBuilderDockWindow("Properties",      right);
		ImGui::DockBuilderDockWindow("Content Browser", bottom);

		// Hide viewport tab bar
		ImGuiDockNode* viewportNode = ImGui::DockBuilderGetNode(center);
		if (viewportNode)
			viewportNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;

		ImGui::DockBuilderFinish(innerID);
	}

	void EditorLayer::DrawMenuBar()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Project", "Ctrl+N"))
					NewProject();

				if (ImGui::MenuItem("Open Project", "Ctrl+O"))
					OpenProject();

				if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
					SaveScene();
				
				if (ImGui::MenuItem("Save Scene As"))
					SaveSceneAs();
				
				if (ImGui::MenuItem("Save All", "Ctrl+Shift+S"))
					SaveProject();
				
				if (ImGui::MenuItem("Exit"))
					Engine::Application::Get().Stop();
				
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

				ImGui::Separator();

				if (ImGui::MenuItem("Reset Layout"))
					m_LevelEditorLayoutBuilt = false;

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
					SaveProject();
				else
					SaveScene();
			}
			break;
		case KeyBoard::Q:
			if (m_ViewportPanel->IsHovered())
				m_ViewportPanel->SetGizmoType(-1);
			break;
		case KeyBoard::W:
			if (m_ViewportPanel->IsHovered())
				m_ViewportPanel->SetGizmoType(ImGuizmo::OPERATION::TRANSLATE);
			break;
		case KeyBoard::E:
			if (m_ViewportPanel->IsHovered())
				m_ViewportPanel->SetGizmoType(ImGuizmo::OPERATION::ROTATE);
			break;
		case KeyBoard::R:
			if (m_ViewportPanel->IsHovered())
				m_ViewportPanel->SetGizmoType(ImGuizmo::OPERATION::SCALE);
			break;
		default: break;
		}
		
		return true;
	}

	bool EditorLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& event)
	{
		// Only pick on LMB when viewport is hovered.
		if (event.GetMouseButton() != Mouse::ButtonLeft)
			return false;

		if (!m_ViewportPanel->IsHovered())
			return false;

		// Don't steal click while camera is flying or gizmo is being dragged.
		if (m_SceneCameraController->GetMode() != SceneCameraMode::None)
			return false;

		if (ImGuizmo::IsOver() || ImGuizmo::IsUsing())
			return false;

		Entity picked = PickEntityAtMouse();
		m_SceneHierarchyPanel->SetSelectedEntity(picked);

		return false; // Don't consume — let ImGui focus the Window.
	}

	void EditorLayer::OpenAssetEditor(const std::filesystem::path& path)
	{
		// Scene — load directly, no tab
		if (path.extension() == ".kscn")
		{
			OpenScene(path);
			return;
		}

		// Check if already open — focus it
		std::string title = path.stem().string();
		for (auto& win : m_OpenWindows)
		{
			if (win->GetTitle() == title)
			{
				ImGui::SetWindowFocus(title.c_str());
				return;
			}
		}

		// Dispatch stub by type
		AssetType type = AssetImporter::DeduceTypeFromPath(path);
		
		switch (type)
		{
			// Stubs for now — replace with real windows as they're built
			case AssetType::Material:
			case AssetType::Texture:
			case AssetType::Mesh:
			case AssetType::Shader:
			default:
				LOG(LogLevel::Info, "OpenAssetEditor: no editor yet for '{}'.", path.string());
				break;
		}
	}

	Entity EditorLayer::PickEntityAtMouse()
	{
		// ---- 1. Mouse → NDC relative to viewport -------------------------
		glm::vec2 mousePos = { ImGui::GetMousePos().x, ImGui::GetMousePos().y };
		glm::vec2 vpMin    = m_ViewportPanel->GetBounds()[0];
		glm::vec2 vpMax    = m_ViewportPanel->GetBounds()[1];
		glm::vec2 vpSize   = vpMax - vpMin;

		if (vpSize.x <= 0.0f || vpSize.y <= 0.0f)
			return {};

		if (mousePos.x < vpMin.x || mousePos.x > vpMax.x ||
			mousePos.y < vpMin.y || mousePos.y > vpMax.y)
			return {};

		float ndcX = ((mousePos.x - vpMin.x) / vpSize.x) * 2.0f - 1.0f;
		float ndcY = 1.0f - ((mousePos.y - vpMin.y) / vpSize.y) * 2.0f;

		// ---------- Build ray ------------------
		const Camera* cam = m_CameraManager.GetActiveCamera();
		if (!cam) return {};

		const glm::mat4& view = cam->GetView();
		const glm::mat4& proj = cam->GetProjection();

		glm::mat4 invView   = glm::inverse(view);
		glm::vec3 rayOrigin = glm::vec3(invView * glm::vec4(0, 0, 0, 1));

		// Unproject NDC point on the near plane to get a view-space direction.
		glm::mat4 invProj  = glm::inverse(proj);
		glm::vec4 viewPos  = invProj * glm::vec4(ndcX, ndcY, 0.0f, 1.0f);
		viewPos /= viewPos.w;

		glm::vec4 viewDir  = glm::vec4(glm::vec3(viewPos), 0.0f);

		glm::vec4 nearPoint = invProj * glm::vec4(ndcX, ndcY, 0.0f, 1.0f);
		glm::vec4 farPoint  = invProj * glm::vec4(ndcX, ndcY, 1.0f, 1.0f);

		nearPoint /= nearPoint.w;
		farPoint  /= farPoint.w;

		glm::vec3 rayDir = glm::normalize(glm::vec3(invView * (farPoint - nearPoint)));

		// Safe inverse — guards divide-by-zero
		// ray component is exactly 0 (axis-aligned views: top, front, etc.).
		auto safeInv = [](float v) -> float
		{
			constexpr float eps = 1e-6f;
			return (glm::abs(v) > eps) ? (1.0f / v) : std::copysign(FLT_MAX, v == 0.0f ? 1.0f : v);
		};
		
		glm::vec3 invDir{ safeInv(rayDir.x), safeInv(rayDir.y), safeInv(rayDir.z) };

		// ---- 3. Collect all hits, sorted front→back ----------------------
		struct Hit { Entity Ent; float T; };
		std::vector<Hit> hits;

		m_ActiveScene->EachEntity([&](Entity entity)
		{
			if (!entity.HasComponent<MeshComponent>() || !entity.HasComponent<TransformComponent>())
				return;

			const auto& mc = entity.GetComponent<MeshComponent>();
			const auto& tc = entity.GetComponent<TransformComponent>();
			if (!mc.HasMesh()) return;

			Mesh::AABB local = mc.MeshRef->ComputeAABB();
			if (!local.IsValid()) return;

			// Transform 8 corners → world-space AABB.
			glm::mat4 model = tc.GetTransform();
			glm::vec3 wMin{  FLT_MAX }, wMax{ -FLT_MAX };
			const glm::vec3 corners[8] = {
				{ local.Min.x, local.Min.y, local.Min.z },
				{ local.Max.x, local.Min.y, local.Min.z },
				{ local.Min.x, local.Max.y, local.Min.z },
				{ local.Max.x, local.Max.y, local.Min.z },
				{ local.Min.x, local.Min.y, local.Max.z },
				{ local.Max.x, local.Min.y, local.Max.z },
				{ local.Min.x, local.Max.y, local.Max.z },
				{ local.Max.x, local.Max.y, local.Max.z },
			};
			for (const auto& c : corners)
			{
				glm::vec3 wc = glm::vec3(model * glm::vec4(c, 1.0f));
				wMin = glm::min(wMin, wc);
				wMax = glm::max(wMax, wc);
			}

			// Slab ray-AABB — safe because invDir has no NaN.
			glm::vec3 t0   = (wMin - rayOrigin) * invDir;
			glm::vec3 t1   = (wMax - rayOrigin) * invDir;
			glm::vec3 tMin = glm::min(t0, t1);
			glm::vec3 tMax = glm::max(t0, t1);

			float tNear = glm::max(glm::max(tMin.x, tMin.y), tMin.z);
			float tFar  = glm::min(glm::min(tMax.x, tMax.y), tMax.z);

			if (tNear > tFar || tFar < 0.0f) return; // miss

			float t = (tNear >= 0.0f) ? tNear : tFar;
			hits.push_back({ entity, t });
		});

		if (hits.empty()) return {};

		// Sort front → back.
		std::sort(hits.begin(), hits.end(), [](const Hit& a, const Hit& b){ return a.T < b.T; });

		// ---- 4. Cycle through overlapping hits ---------------------------
		// If the currently selected entity is the front hit, return the next
		// one behind it so repeated clicks cycle through overlapping objects.
		Entity currentSelection = m_SceneHierarchyPanel->GetSelectedEntity();

		if (currentSelection && hits.size() > 1)
		{
			// Find selected entity in the hit list.
			for (size_t i = 0; i < hits.size(); ++i)
			{
				if (hits[i].Ent == currentSelection)
					return hits[(i + 1) % hits.size()].Ent; // cycle
			}
		}

		return hits.front().Ent; // default: closest
	}

	void EditorLayer::NewProject()
	{
		Project::New();
	}

	void EditorLayer::OpenProject()
	{
		auto projectPath = PlatformUtils::OpenFileDialog(KPROJ_FILTER);
		
		if (projectPath.empty())
		{
			Application::Get().Stop();
			return;
		}
		
		if (Project::Load(projectPath))
		{
			auto am= CreateRef<EditorAssetManager>();
			Project::GetActive()->SetAssetManager(am);
			
			auto startScenePath = Project::GetAssetPath(Project::GetActive()->GetConfig().StartupScene);
			OpenScene(startScenePath);
			
			m_SceneHierarchyPanel = CreateScope<SceneHierarchyPanel>();
			m_ContentBrowserPanel = CreateScope<ContentBrowserPanel>();
			m_PropertiesPanel     = CreateScope<PropertiesPanel>();
			m_ViewportPanel       = CreateScope<ViewportPanel>();

			m_ContentBrowserPanel->OnAssetDoubleClicked = [this](const std::filesystem::path& path)
			{
				OpenAssetEditor(path);
			};
		}
	}

	void EditorLayer::SaveProject()
	{
	}
	
	void EditorLayer::OpenScene(const std::filesystem::path& filepath)
	{
		// Clear previous scene — reset registry by replacing with a fresh instance
		m_ActiveScene = CreateRef<Scene>();
		if (m_SceneHierarchyPanel)
			m_SceneHierarchyPanel->SetContext(m_ActiveScene);

		SceneSerializer serializer(m_ActiveScene);
		if (serializer.Deserialize(filepath.string()))
			m_ActiveScenePath = filepath;
	}
	
	void EditorLayer::SaveScene()
	{
		SceneSerializer serializer(m_ActiveScene);
		serializer.Serialize(m_ActiveScenePath.string());
		
		LOG(LogLevel::Info, "Saving Scene");
	}

	void EditorLayer::SaveSceneAs()
	{
		auto path = PlatformUtils::SaveFileDialog("Scene\0*.kscn\0\0", "Untitled.kscn");
		
		if (!path.empty())
			SceneSerializer(m_ActiveScene).Serialize(path.string());
	}
}

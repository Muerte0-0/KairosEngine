#include "EditorLayer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>

#include "imgui_internal.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Engine/Assets/Editor/AssetImporter.h"
#include "Windows/TextureEditorWindow.h"
#include "Windows/MeshEditorWindow.h"

#include "Engine/Scene/SceneSerializer.h"
#include "Engine/Scene/Prefab.h"

#include "ImGuizmo.h"
#include "Windows/MaterialEditorWindow.h"
#include "Engine/Core/LoadingSystem.h"

constexpr const char* KPROJ_FILTER = "Kairos Project\0*.kproj\0\0";

namespace
{
	struct PrefabOverrideSnapshot
	{
		std::unordered_set<Engine::ComponentType> Types;

		Engine::TagComponent       Tag;
		Engine::TransformComponent Transform;
		Engine::MeshComponent      Mesh;
		Engine::CameraComponent    Camera;
		Engine::LightComponent     Light;

		bool HasTag       = false;
		bool HasTransform = false;
		bool HasMesh      = false;
		bool HasCamera    = false;
		bool HasLight     = false;
	};

	PrefabOverrideSnapshot CapturePrefabOverrides(Engine::Entity entity)
	{
		PrefabOverrideSnapshot snapshot;
		if (!entity || !entity.HasComponent<Engine::PrefabOverrideComponent>())
			return snapshot;

		snapshot.Types = entity.GetComponent<Engine::PrefabOverrideComponent>().OverriddenComponents;
		if (snapshot.Types.contains(Engine::ComponentType::Tag) && entity.HasComponent<Engine::TagComponent>())
		{
			snapshot.Tag = entity.GetComponent<Engine::TagComponent>();
			snapshot.HasTag = true;
		}
		if (snapshot.Types.contains(Engine::ComponentType::Transform) && entity.HasComponent<Engine::TransformComponent>())
		{
			snapshot.Transform = entity.GetComponent<Engine::TransformComponent>();
			snapshot.HasTransform = true;
		}
		if (snapshot.Types.contains(Engine::ComponentType::Mesh) && entity.HasComponent<Engine::MeshComponent>())
		{
			snapshot.Mesh = entity.GetComponent<Engine::MeshComponent>();
			snapshot.HasMesh = true;
		}
		if (snapshot.Types.contains(Engine::ComponentType::Camera) && entity.HasComponent<Engine::CameraComponent>())
		{
			snapshot.Camera = entity.GetComponent<Engine::CameraComponent>();
			snapshot.HasCamera = true;
		}
		if (snapshot.Types.contains(Engine::ComponentType::Light) && entity.HasComponent<Engine::LightComponent>())
		{
			snapshot.Light = entity.GetComponent<Engine::LightComponent>();
			snapshot.HasLight = true;
		}

		return snapshot;
	}

	template<typename T>
	void ReplaceOrRemoveComponent(Engine::Entity entity, bool hasComponent, const T& component)
	{
		auto& registry = entity.GetScene()->GetRegistry();
		auto enttID = static_cast<entt::entity>(entity);
		if (hasComponent)
			registry.emplace_or_replace<T>(enttID, component);
		else if (entity.HasComponent<T>())
			entity.RemoveComponent<T>();
	}

	void ApplyPrefabOverrideSnapshot(Engine::Entity entity, const PrefabOverrideSnapshot& snapshot)
	{
		if (!entity)
			return;

		if (snapshot.Types.contains(Engine::ComponentType::Tag))
			ReplaceOrRemoveComponent(entity, snapshot.HasTag, snapshot.Tag);
		if (snapshot.Types.contains(Engine::ComponentType::Transform))
			ReplaceOrRemoveComponent(entity, snapshot.HasTransform, snapshot.Transform);
		if (snapshot.Types.contains(Engine::ComponentType::Mesh))
			ReplaceOrRemoveComponent(entity, snapshot.HasMesh, snapshot.Mesh);
		if (snapshot.Types.contains(Engine::ComponentType::Camera))
			ReplaceOrRemoveComponent(entity, snapshot.HasCamera, snapshot.Camera);
		if (snapshot.Types.contains(Engine::ComponentType::Light))
			ReplaceOrRemoveComponent(entity, snapshot.HasLight, snapshot.Light);

		if (!snapshot.Types.empty())
		{
			if (!entity.HasComponent<Engine::PrefabOverrideComponent>())
				entity.AddComponent<Engine::PrefabOverrideComponent>();
			entity.GetComponent<Engine::PrefabOverrideComponent>().OverriddenComponents = snapshot.Types;
		}
	}

	Engine::EntityID GetParentEntityID(Engine::Entity entity)
	{
		if (!entity)
			return Engine::INVALID_ENTITY;

		const Engine::SceneNode* node = entity.GetScene()->GetSceneGraph().GetNode(static_cast<Engine::EntityID>(entity));
		return node ? node->Parent : Engine::INVALID_ENTITY;
	}

	void RestoreParent(Engine::Entity entity, Engine::EntityID parent)
	{
		if (!entity || parent == Engine::INVALID_ENTITY)
			return;

		auto parentEntt = static_cast<entt::entity>(parent);
		if (entity.GetScene()->GetRegistry().valid(parentEntt))
			entity.GetScene()->GetSceneGraph().SetParent(static_cast<Engine::EntityID>(entity), parent);
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

		LoadingSystem::SetStatusTextMainThread("Select project...");
		LoadingSystem::SetStartupProgressMainThread(0.60f);
		OpenProject();
		
		m_SceneHierarchyPanel->SetContext(m_ActiveScene);

		m_ViewportPanel->Init(m_SceneRenderer.get(), m_SceneCamera.get(),
		                      m_SceneCameraController.get(), &m_CameraManager,
		                      m_SceneHierarchyPanel.get());

		m_ViewportPanel->OnSceneDrop = [this](const std::filesystem::path& path)
		{
			m_ActiveScenePath = path;
			Application::Get().RequestSceneChange(path.string());
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

		m_ViewportPanel->OnPrefabDrop = [this](const std::filesystem::path& path)
		{
			auto* editorAM = static_cast<EditorAssetManager*>(
				Project::GetActive()->GetAssetManager().get());

			AssetHandle handle = editorAM->ImportAsset(path);
			if (static_cast<uint64_t>(handle) == NullAssetHandle)
			{
				LOG(LogLevel::Warning, "Prefab drag-drop: import failed for '{}'.", path.string());
				return;
			}
			Ref<Prefab> prefab = AssetManager::GetAsset<Prefab>(handle);
			if (!prefab)
			{
				LOG(LogLevel::Warning, "Prefab drag-drop: load failed for '{}'.", path.string());
				return;
			}
			Entity root = prefab->Instantiate(m_ActiveScene.get());
			if (root && m_SceneHierarchyPanel)
				m_SceneHierarchyPanel->SetSelectedEntity(root);
			LOG(LogLevel::Info, "Prefab '{}' instantiated via drag-drop.", path.stem().string());
		};
	}

	void EditorLayer::OnDetach()
	{
		Layer::OnDetach();
	}

	void EditorLayer::OnUpdate(float DeltaTime)
	{
		Layer::OnUpdate(DeltaTime);

		// When editing a prefab use the isolated scene; otherwise use the active scene.
		Scene* renderScene = m_IsEditingPrefab ? m_PrefabEditingScene.get() : m_ActiveScene.get();

		renderScene->OnUpdate(DeltaTime);
		m_CameraManager.UpdateFromRegistry(renderScene->GetRegistry());

		m_SceneCameraController->SetViewportFocused(m_ViewportPanel->IsFocused());
		m_SceneCameraController->SetViewportHovered(m_ViewportPanel->IsHovered());
		
		m_SceneCameraController->OnUpdate(DeltaTime);
	}

	void EditorLayer::OnFixedUpdate(float DeltaTime)
	{
		Layer::OnFixedUpdate(DeltaTime);
		
		if (m_SceneCameraController->GetMode() != SceneCameraMode::None)
			Input::SetCursorLockMode(CursorMode::Locked);
		else
			Input::SetCursorLockMode(CursorMode::Normal);
	}

	void EditorLayer::OnRender()
	{
		Layer::OnRender();

		if (!m_SceneRenderer)
			return;

		Scene* renderScene = m_IsEditingPrefab ? m_PrefabEditingScene.get() : m_ActiveScene.get();
		m_SceneRenderer->BeginScene(m_CameraManager);
		renderScene->OnRender(*m_SceneRenderer);
		m_SceneRenderer->EndScene();
	}

	void EditorLayer::OnImGuiRender()
	{
		SetupOuterDockspace();
		DrawLevelEditorWindow();

		// Tool windows (Material Editor, Texture Editor, etc.)
		for (auto it = m_OpenWindows.begin(); it != m_OpenWindows.end();)
		{
			(*it)->SetOuterDockID(m_OuterDockID);
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
		// Scene name displayed inside the window instead
		ImGuiCond dockCond = m_LevelEditorLayoutBuilt ? ImGuiCond_FirstUseEver : ImGuiCond_Always;
		ImGui::SetNextWindowDockID(m_OuterDockID, dockCond);
		ImGui::Begin("Level Editor", nullptr, ImGuiWindowFlags_NoCollapse);

		// Show scene name as text inside the window
		std::string sceneName = m_ActiveScenePath.empty()
			? "Untitled"
			: m_ActiveScene->GetName();
		ImGui::TextDisabled("Scene: %s", sceneName.c_str());
		ImGui::SameLine();

		ImGui::Dummy({ImGui::GetContentRegionAvail().x / 2.5f, 0.0f});
		
		const EngineMode mode = Engine::Application::Get().GetEngineMode();

		// Play button (Editor mode only)
		if (mode == EngineMode::Editor)
		{
			ImGui::SameLine();
			if (ImGui::Button("Play")) // u8"\u25b6"
				Engine::Application::Get().EnterPlayMode();
		}

		// Pause button (Play mode only)
		if (mode == EngineMode::Play)
		{
			ImGui::SameLine();
			if (ImGui::Button("Pause")) // u8"\u23f8"
				Engine::Application::Get().SetEngineMode(EngineMode::Paused);
		}

		// Resume button (Paused mode only)
		if (mode == EngineMode::Paused)
		{
			ImGui::SameLine();
			if (ImGui::Button("Resume")) // u8"\u25b6"
				Engine::Application::Get().SetEngineMode(EngineMode::Play);
		}

		// Stop button (Play or Paused)
		if (mode == EngineMode::Play || mode == EngineMode::Paused)
		{
			ImGui::SameLine();
			if (ImGui::Button("Stop")) // u8"\u23f9"
				Engine::Application::Get().ExitPlayMode();
		}
		
		ImGui::Separator();

		// Prefab editor banner
		if (m_IsEditingPrefab)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
			ImGui::TextUnformatted("  [PREFAB EDIT MODE]");
			ImGui::PopStyleColor();
			ImGui::SameLine();
			if (ImGui::Button("Save & Apply"))
				SavePrefabAndApply();
			ImGui::SameLine();
			if (ImGui::Button("Exit"))
				ExitPrefabEditor();
			ImGui::Separator();
		}

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

	void EditorLayer::DrawDebugWindow()
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
		// Scene — load async, no tab
		if (path.extension() == ".kscn")
		{
			m_ActiveScenePath = path;
			Application::Get().RequestSceneChange(path.string());
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

		// Dispatch by type
		AssetType type = AssetImporter::DeduceTypeFromPath(path);

		switch (type)
		{
			case AssetType::Texture:
			{
				auto editorAM = Project::GetActive()->GetEditorAssetManager();
				AssetHandle handle = editorAM->ImportAsset(path);
				m_OpenWindows.push_back(CreateRef<TextureEditorWindow>(path, handle));
				break;
			}
			case AssetType::Material:
			{
				auto editorAM = Project::GetActive()->GetEditorAssetManager();
				AssetHandle handle = editorAM->ImportAsset(path);
				m_OpenWindows.push_back(CreateRef<MaterialEditorWindow>(path, handle));
				break;
			}
			case AssetType::Mesh:
			{
				auto editorAM = Project::GetActive()->GetEditorAssetManager();
				AssetHandle handle = editorAM->ImportAsset(path);
				m_OpenWindows.push_back(CreateRef<MeshEditorWindow>(path, handle));
				break;
			}
			case AssetType::Shader:
				LOG(LogLevel::Info, "OpenAssetEditor: Shader editor not yet implemented.");
				break;
			case AssetType::Prefab:
			{
				// Double-click → open prefab editor (not instantiate)
				auto editorAM = Project::GetActive()->GetEditorAssetManager();
				AssetHandle handle = editorAM->ImportAsset(path);
				if (static_cast<uint64_t>(handle) == NullAssetHandle)
				{
					LOG(LogLevel::Warning, "OpenAssetEditor: prefab import failed for '{}'.", path.string());
					break;
				}
				OpenPrefabEditor(handle);
				break;
			}
			default:
				LOG(LogLevel::Warning, "OpenAssetEditor: unknown type for '{}'.", path.string());
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
		LoadingSystem::SetStatusTextMainThread("Opening project...");
		LoadingSystem::SetStartupProgressMainThread(0.62f);
		auto projectPath = PlatformUtils::OpenFileDialog(KPROJ_FILTER);
		
		if (projectPath.empty())
		{
			Application::Get().Stop();
			return;
		}
		
		if (Project::Load(projectPath))
		{
			LoadingSystem::SetStatusTextMainThread("Scanning assets...");
			LoadingSystem::SetStartupProgressMainThread(0.65f);
			auto am= CreateRef<EditorAssetManager>();
			Project::GetActive()->SetAssetManager(am);
			
			auto startScenePath = Project::GetAssetPath(Project::GetActive()->GetConfig().StartupScene);
			LoadingSystem::SetStatusTextMainThread("Deserializing startup scene...");
			LoadingSystem::SetStartupProgressMainThread(0.85f);
			OpenScene(startScenePath);
			
			m_SceneHierarchyPanel = CreateScope<SceneHierarchyPanel>();
			m_ContentBrowserPanel = CreateScope<ContentBrowserPanel>();
			m_PropertiesPanel     = CreateScope<PropertiesPanel>();
			m_ViewportPanel       = CreateScope<ViewportPanel>();

			m_ContentBrowserPanel->OnAssetDoubleClicked = [this](const std::filesystem::path& path)
			{
				OpenAssetEditor(path);
			};

			m_ContentBrowserPanel->OnAssetSelected = [this](const std::filesystem::path& path)
			{
				auto editorAM = Engine::Project::GetActive()->GetEditorAssetManager();
				Engine::AssetHandle handle = editorAM->ImportAsset(path);
				m_PropertiesPanel->SetSelectedAsset(handle);
				
				for (auto& win : m_OpenWindows)
					if (auto* matWin = dynamic_cast<MaterialEditorWindow*>(win.get()))
						matWin->SetContentBrowserSelection(path);
			};

			// ── Prefab: instantiate from content browser ──────────────────────
			m_ContentBrowserPanel->OnPrefabInstantiate = [this](const std::filesystem::path& path)
			{
				auto editorAM = Engine::Project::GetActive()->GetEditorAssetManager();
				Engine::AssetHandle handle = editorAM->ImportAsset(path);
				if (static_cast<uint64_t>(handle) == Engine::NullAssetHandle)
				{
					LOG(Engine::LogLevel::Warning, "Prefab instantiate: import failed for '{}'.", path.string());
					return;
				}

				Engine::Ref<Engine::Prefab> prefab = Engine::AssetManager::GetAsset<Engine::Prefab>(handle);
				if (!prefab)
				{
					LOG(Engine::LogLevel::Warning, "Prefab instantiate: load failed for '{}'.", path.string());
					return;
				}

				Engine::Entity root = prefab->Instantiate(m_ActiveScene.get());
				if (root && m_SceneHierarchyPanel)
					m_SceneHierarchyPanel->SetSelectedEntity(root);

				LOG(Engine::LogLevel::Info, "Prefab '{}' instantiated.", path.stem().string());
			};

			// ── Prefab: save entity hierarchy from scene hierarchy panel ──────
			m_SceneHierarchyPanel->OnSaveAsPrefab = [this](Engine::Entity entity)
			{
				// Build a unique filename in the content browser's current directory
				std::string baseName = entity.GetComponent<Engine::TagComponent>().Tag;
				std::filesystem::path dir = m_ContentBrowserPanel->GetCurrentDirectory();

				std::filesystem::path candidate = dir / (baseName + ".prefab");
				int suffix = 0;
				while (std::filesystem::exists(candidate))
					candidate = dir / (baseName + std::to_string(++suffix) + ".prefab");

				Engine::PrefabData data = Engine::SerializeEntityHierarchy(entity);
				if (!Engine::SavePrefab(candidate, data))
				{
					LOG(Engine::LogLevel::Error, "Failed to save prefab '{}'.", candidate.string());
					return;
				}

				// Register with asset manager so it's tracked
				auto editorAM = Engine::Project::GetActive()->GetEditorAssetManager();
				editorAM->ImportAsset(candidate);

				// Trigger inline rename in content browser — same UX as CreateNewAsset
				m_ContentBrowserPanel->BeginRename(candidate);

				LOG(Engine::LogLevel::Info, "Saved prefab '{}'.", candidate.stem().string());
			};

			// ── Prefab: revert instance ───────────────────────────────────────
			m_SceneHierarchyPanel->OnRevertPrefabInstance = [this](Engine::Entity entity)
			{
				RevertPrefabInstance(entity);
			};

			// ── Prefab: open prefab editor ────────────────────────────────────
			m_SceneHierarchyPanel->OnOpenPrefabEditor = [this](Engine::AssetHandle handle)
			{
				OpenPrefabEditor(handle);
			};

			// ── Prefab: dirty tracking ─────────────────────────────────────────
			m_PropertiesPanel->OnEntityModified = [this]()
			{
				if (m_IsEditingPrefab)
					m_PrefabDirty = true;
			};
			m_PropertiesPanel->OnApplyPrefabInstance = [this](Engine::Entity entity)
			{
				ApplyPrefabInstanceToPrefab(entity);
			};
			m_PropertiesPanel->OnRevertPrefabInstance = [this](Engine::Entity entity)
			{
				RevertPrefabInstance(entity);
			};
		}
	}

	void EditorLayer::SaveProject()
	{
	}

	void EditorLayer::OnSceneLoaded(const Ref<Scene>& scene)
	{
		m_ActiveScene = scene;
		Application::Get().SetEditorScene(scene);
		if (m_SceneHierarchyPanel)
			m_SceneHierarchyPanel->SetContext(m_ActiveScene);
	}

	void EditorLayer::OnEnterPlayMode(const Ref<Scene>& runtimeScene)
	{
		m_ActiveScene = runtimeScene;
		if (m_SceneHierarchyPanel)
			m_SceneHierarchyPanel->SetContext(m_ActiveScene);
	}

	void EditorLayer::OnExitPlayMode(const Ref<Scene>& editorScene)
	{
		m_ActiveScene = editorScene;
		if (m_SceneHierarchyPanel)
			m_SceneHierarchyPanel->SetContext(m_ActiveScene);
	}
	
	void EditorLayer::OpenScene(const std::filesystem::path& filepath)
	{
		LoadingSystem::SetStatusTextMainThread("Loading scene (deserialize + assets): " + filepath.stem().string() + "...");
		// Clear previous scene — reset registry by replacing with a fresh instance
		m_ActiveScene = CreateRef<Scene>();
		if (m_SceneHierarchyPanel)
			m_SceneHierarchyPanel->SetContext(m_ActiveScene);

		SceneSerializer serializer(m_ActiveScene);
		if (serializer.Deserialize(filepath.string()))
		{
			m_ActiveScenePath = filepath;
			m_ActiveScene->SetName(filepath.stem().string());
			// Allow incremental startup pump to resolve scene asset handles to live GPU resources.
			LoadingSystem::SetStartupScene(m_ActiveScene);
			Application::Get().SetEditorScene(m_ActiveScene);
		}
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

	// ---------------------------------------------------------------------------
	// Prefab Editor
	// ---------------------------------------------------------------------------

	void EditorLayer::OpenPrefabEditor(Engine::AssetHandle prefabHandle)
	{
		if (m_IsEditingPrefab)
			ExitPrefabEditor();

		Engine::Ref<Engine::Prefab> prefab = Engine::AssetManager::GetAsset<Engine::Prefab>(prefabHandle);
		if (!prefab)
		{
			LOG(Engine::LogLevel::Warning, "OpenPrefabEditor: failed to load prefab asset.");
			return;
		}

		// Create isolated scene and instantiate prefab into it
		m_PrefabEditingScene = CreateRef<Scene>();
		m_PrefabEditingScene->SetName("PrefabEdit");
		prefab->Instantiate(m_PrefabEditingScene.get());

		m_EditingPrefab   = prefabHandle;
		m_IsEditingPrefab = true;
		m_PrefabDirty     = false;

		if (m_SceneHierarchyPanel)
			m_SceneHierarchyPanel->SetContext(m_PrefabEditingScene);

		LOG(Engine::LogLevel::Info, "Opened prefab editor.");
	}

	void EditorLayer::SavePrefabAndApply()
	{
		if (!m_IsEditingPrefab || !m_PrefabEditingScene)
			return;

		// 1. Find root entity in prefab scene (entity with PrefabInstanceComponent)
		Engine::Entity prefabRoot;
		m_PrefabEditingScene->EachEntity([&](Engine::Entity e)
		{
			if (e.HasComponent<Engine::PrefabInstanceComponent>())
				prefabRoot = e;
		});

		if (!prefabRoot)
		{
			LOG(Engine::LogLevel::Warning, "SavePrefabAndApply: no root entity found.");
			return;
		}

		// 2. Serialize prefab scene → PrefabData
		Engine::PrefabData newData = Engine::SerializeEntityHierarchy(prefabRoot);

		// 3. Find source path and overwrite .prefab file
		auto* editorAM = static_cast<EditorAssetManager*>(
			Engine::Project::GetActive()->GetAssetManager().get());

		const Engine::AssetMetadata* meta = editorAM->GetRegistry().Get(m_EditingPrefab);
		if (!meta || !meta->IsValid())
		{
			LOG(Engine::LogLevel::Error, "SavePrefabAndApply: prefab handle not in registry.");
			return;
		}

		std::filesystem::path sourcePath = Engine::Project::GetAssetPath(meta->FilePath);
		if (!Engine::SavePrefab(sourcePath, newData))
		{
			LOG(Engine::LogLevel::Error, "SavePrefabAndApply: write failed.");
			return;
		}

		// 4. Reload asset so GetAsset<Prefab> returns updated data
		editorAM->ReimportAsset(m_EditingPrefab);
		Engine::Ref<Engine::Prefab> updatedPrefab = Engine::AssetManager::GetAsset<Engine::Prefab>(m_EditingPrefab);
		if (!updatedPrefab)
		{
			LOG(Engine::LogLevel::Error, "SavePrefabAndApply: reload failed.");
			return;
		}
		updatedPrefab->Data = newData;

		// 5. Apply to all instances in active scene
		Engine::AssetHandle handle = m_EditingPrefab;
		std::vector<Engine::Entity> instances;
		m_ActiveScene->EachEntity([&](Engine::Entity e)
		{
			if (e.HasComponent<Engine::PrefabInstanceComponent>())
				if (e.GetComponent<Engine::PrefabInstanceComponent>().PrefabHandle == handle)
					instances.push_back(e);
		});

		for (Engine::Entity inst : instances)
		{
			PrefabOverrideSnapshot overrides = CapturePrefabOverrides(inst);
			Engine::EntityID parent = GetParentEntityID(inst);

			m_ActiveScene->DestroyEntityHierarchy(inst);
			Engine::Entity newEnt = updatedPrefab->Instantiate(m_ActiveScene.get());
			RestoreParent(newEnt, parent);
			ApplyPrefabOverrideSnapshot(newEnt, overrides);
		}

		m_PrefabDirty = false;
		LOG(Engine::LogLevel::Info, "Prefab saved and applied to {} instance(s).", instances.size());
	}

	void EditorLayer::ApplyPrefabInstanceToPrefab(Engine::Entity entity)
	{
		if (!entity || !entity.HasComponent<Engine::PrefabInstanceComponent>())
			return;

		Engine::AssetHandle handle = entity.GetComponent<Engine::PrefabInstanceComponent>().PrefabHandle;
		Engine::PrefabData newData = Engine::SerializeEntityHierarchy(entity);

		auto* editorAM = static_cast<EditorAssetManager*>(
			Engine::Project::GetActive()->GetAssetManager().get());

		const Engine::AssetMetadata* meta = editorAM->GetRegistry().Get(handle);
		if (!meta || !meta->IsValid())
		{
			LOG(Engine::LogLevel::Error, "ApplyPrefabInstanceToPrefab: prefab handle not in registry.");
			return;
		}

		std::filesystem::path sourcePath = Engine::Project::GetAssetPath(meta->FilePath);
		if (!Engine::SavePrefab(sourcePath, newData))
		{
			LOG(Engine::LogLevel::Error, "ApplyPrefabInstanceToPrefab: write failed.");
			return;
		}

		editorAM->ReimportAsset(handle);
		Engine::Ref<Engine::Prefab> updatedPrefab = Engine::AssetManager::GetAsset<Engine::Prefab>(handle);
		if (!updatedPrefab)
		{
			LOG(Engine::LogLevel::Error, "ApplyPrefabInstanceToPrefab: reload failed.");
			return;
		}
		updatedPrefab->Data = newData;

		std::vector<Engine::Entity> instances;
		m_ActiveScene->EachEntity([&](Engine::Entity e)
		{
			if (e.HasComponent<Engine::PrefabInstanceComponent>() &&
				e.GetComponent<Engine::PrefabInstanceComponent>().PrefabHandle == handle)
				instances.push_back(e);
		});

		for (Engine::Entity inst : instances)
		{
			PrefabOverrideSnapshot overrides = CapturePrefabOverrides(inst);
			Engine::EntityID parent = GetParentEntityID(inst);
			m_ActiveScene->DestroyEntityHierarchy(inst);
			Engine::Entity newEnt = updatedPrefab->Instantiate(m_ActiveScene.get());
			RestoreParent(newEnt, parent);
			ApplyPrefabOverrideSnapshot(newEnt, overrides);
			if (m_SceneHierarchyPanel && inst == entity)
				m_SceneHierarchyPanel->SetSelectedEntity(newEnt);
		}

		LOG(Engine::LogLevel::Info, "Prefab instance applied to prefab and propagated to {} instance(s).", instances.size());
	}

	void EditorLayer::RevertPrefabInstance(Engine::Entity entity)
	{
		if (!entity || !entity.HasComponent<Engine::PrefabInstanceComponent>())
			return;

		Engine::AssetHandle handle = entity.GetComponent<Engine::PrefabInstanceComponent>().PrefabHandle;
		Engine::Ref<Engine::Prefab> prefab = Engine::AssetManager::GetAsset<Engine::Prefab>(handle);
		if (!prefab)
		{
			LOG(Engine::LogLevel::Warning, "RevertPrefabInstance: prefab asset not found.");
			return;
		}

		Engine::TransformComponent savedTransform = entity.GetComponent<Engine::TransformComponent>();
		Engine::EntityID parent = GetParentEntityID(entity);

		m_ActiveScene->DestroyEntityHierarchy(entity);
		if (m_SceneHierarchyPanel)
			m_SceneHierarchyPanel->SetSelectedEntity({});

		Engine::Entity newEnt = prefab->Instantiate(m_ActiveScene.get());
		if (newEnt)
		{
			RestoreParent(newEnt, parent);
			auto& newTc = newEnt.GetComponent<Engine::TransformComponent>();
			newTc.Translation = savedTransform.Translation;
			newTc.Rotation    = savedTransform.Rotation;
			newTc.Scale       = savedTransform.Scale;
			if (newEnt.HasComponent<Engine::PrefabOverrideComponent>())
				newEnt.RemoveComponent<Engine::PrefabOverrideComponent>();

			if (m_SceneHierarchyPanel)
				m_SceneHierarchyPanel->SetSelectedEntity(newEnt);
		}

		LOG(Engine::LogLevel::Info, "Prefab instance reverted.");
	}

	void EditorLayer::ExitPrefabEditor()
	{
		if (!m_IsEditingPrefab)
			return;

		if (m_PrefabDirty)
			SavePrefabAndApply();

		m_PrefabEditingScene.reset();
		m_EditingPrefab   = Engine::AssetHandle(Engine::NullAssetHandle);
		m_IsEditingPrefab = false;
		m_PrefabDirty     = false;

		if (m_SceneHierarchyPanel)
			m_SceneHierarchyPanel->SetContext(m_ActiveScene);

		LOG(Engine::LogLevel::Info, "Exited prefab editor.");
	}
}

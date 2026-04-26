#include "SceneHierarchyPanel.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "Engine/Scene/Components.h"
#include "Engine/Scene/SceneGraph.h"
#include "Engine/Utils/PrimitiveMeshFactory.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Kairos
{
	// ---------------------------------------------------------------------------
	// Decompose a mat4 back into TRS. glm::decompose output is in weird order so
	// we wrap it for clarity.
	// ---------------------------------------------------------------------------
	static void DecomposeTransform(const glm::mat4& transform,
		glm::vec3& outTranslation,
		glm::vec3& outRotationEuler,
		glm::vec3& outScale)
	{
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::quat orientation;
		glm::decompose(transform, outScale, orientation, outTranslation, skew, perspective);
		outRotationEuler = glm::eulerAngles(orientation);
	}

	// ---------------------------------------------------------------------------
	// Reparent childID under newParentID while keeping child's world position.
	// Pass INVALID_ENTITY as newParentID to promote to root.
	// ---------------------------------------------------------------------------
	static void ReparentPreserveWorld(Engine::SceneGraph& graph,
		Engine::EntityID childID,
		Engine::EntityID newParentID,
		Engine::Scene* scene)
	{
		auto& tc = Entity{ childID, scene }.GetComponent<Engine::TransformComponent>();

		// Cache current world (already propagated this frame)
		glm::mat4 childWorld = tc.WorldTransform;

		// Get new parent world
		glm::mat4 newParentWorld = glm::mat4(1.0f);
		if (newParentID != Engine::INVALID_ENTITY)
		{
			auto& parentTC = Entity{ newParentID, scene }.GetComponent<Engine::TransformComponent>();
			newParentWorld = parentTC.WorldTransform;
		}

		// Apply new relationship
		if (newParentID == Engine::INVALID_ENTITY)
			graph.RemoveParent(childID);
		else
			graph.SetParent(childID, newParentID);

		// Recompute local = inverse(newParentWorld) * childWorld
		glm::mat4 newLocal = glm::inverse(newParentWorld) * childWorld;
		DecomposeTransform(newLocal, tc.Translation, tc.Rotation, tc.Scale);

		// WorldTransform will be recomputed by PropagateTransforms next frame.
		// Set it now too so the frame it's reparented doesn't flicker.
		tc.WorldTransform = childWorld;
	}

	// ---------------------------------------------------------------------------

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		// Drop onto empty panel space → promote to root
		if (ImGui::BeginDragDropTargetCustom(ImGui::GetCurrentWindow()->InnerRect, ImGui::GetID("SceneHierarchyRoot")))
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_ENTITY"))
			{
				Engine::EntityID dropped = *(Engine::EntityID*)payload->Data;
				Engine::SceneGraph& graph = m_Context->GetSceneGraph();
				Engine::SceneNode* node = graph.GetNode(dropped);
				if (node && node->Parent != Engine::INVALID_ENTITY)
					ReparentPreserveWorld(graph, dropped, Engine::INVALID_ENTITY, m_Context.get());
			}
			ImGui::EndDragDropTarget();
		}

		for (Engine::EntityID root : m_Context->GetSceneGraph().GetRootNodes())
		{
			Entity e{ root, m_Context.get() };
			if (e) DrawEntityNode(e);
		}

		if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
			m_SelectionContext = {};

		ImGuiPopupFlags flags = ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight;

		if (ImGui::BeginPopupContextWindow(nullptr, flags))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
				m_Context->CreateEntity("Empty Entity");

			ImGui::Separator();

			if (ImGui::BeginMenu("Mesh Entity"))
			{
				auto CreatePrimitive = [&](const char* name, const char* key)
				{
					Engine::Entity e = m_Context->CreateEntity(name);
					auto& mc = e.AddComponent<Engine::MeshComponent>();
					mc.SetPrimitiveMesh(key, Engine::PrimitiveMeshFactory::GetOrCreate(key));
					m_SelectionContext = e;
				};

				if (ImGui::MenuItem("Cube"))   CreatePrimitive("Cube",   Engine::PrimitiveKey::Cube);
				if (ImGui::MenuItem("Plane"))  CreatePrimitive("Plane",  Engine::PrimitiveKey::Plane);
				if (ImGui::MenuItem("Sphere")) CreatePrimitive("Sphere", Engine::PrimitiveKey::Sphere);
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Light Entity"))
			{
				auto CreateLight = [&](const char* name, Engine::LightType type)
				{
					Engine::Entity e = m_Context->CreateEntity(name);
					auto& lc = e.AddComponent<Engine::LightComponent>();
					lc.Type = type;
					m_SelectionContext = e;
				};

				if (ImGui::MenuItem("Directional Light")) CreateLight("Directional Light", Engine::LightType::Directional);
				if (ImGui::MenuItem("Point Light"))       CreateLight("Point Light",        Engine::LightType::Point);
				if (ImGui::MenuItem("Spot Light"))        CreateLight("Spot Light",         Engine::LightType::Spot);
				ImGui::EndMenu();
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Camera"))
			{
				Engine::Entity e = m_Context->CreateEntity("Camera");
				e.AddComponent<Engine::CameraComponent>();
				m_SelectionContext = e;
			}

			ImGui::EndPopup();
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<Engine::TagComponent>();
		Engine::EntityID id = entity;
		Engine::SceneGraph& graph = m_Context->GetSceneGraph();
		Engine::SceneNode* node = graph.GetNode(id);

		ImGuiTreeNodeFlags flags =
			ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_SpanAvailWidth |
			ImGuiTreeNodeFlags_FramePadding;

		if (m_SelectionContext == entity)
			flags |= ImGuiTreeNodeFlags_Selected;

		bool hasChildren = node && !node->Children.empty();
		if (!hasChildren)
			flags |= ImGuiTreeNodeFlags_Leaf;

		bool opened = ImGui::TreeNodeEx((void*)(intptr_t)(uint32_t)(entt::entity)entity,
			flags, "%s", tag.Tag.c_str());

		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			m_SelectionContext = entity;

		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
			{
				m_Context->DestroyEntity(entity);
				if (m_SelectionContext == entity) m_SelectionContext = {};
				if (opened) ImGui::TreePop();
				ImGui::EndPopup();
				return;
			}

			if (ImGui::MenuItem("Create Child"))
			{
				// New entity has WorldTransform = identity (just created, propagation hasn't run).
				// SetParent directly — local stays at (0,0,0) which IS correct world-relative
				// since it's a brand-new entity with no prior world position.
				Entity child = m_Context->CreateEntity("Child Entity");
				Engine::EntityID childID = child;
				graph.SetParent(childID, id);
				m_SelectionContext = child;
			}

			ImGui::EndPopup();
		}

		// --- Drag source ---
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			ImGui::SetDragDropPayload("SCENE_ENTITY", &id, sizeof(Engine::EntityID));
			ImGui::Text("Move: %s", tag.Tag.c_str());
			ImGui::EndDragDropSource();
		}

		// --- Drop target: reparent onto this node ---
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_ENTITY"))
			{
				Engine::EntityID droppedID = *(Engine::EntityID*)payload->Data;

				if (droppedID != id && !graph.IsAncestor(droppedID, id))
					ReparentPreserveWorld(graph, droppedID, id, m_Context.get());
			}
			ImGui::EndDragDropTarget();
		}

		if (opened)
		{
			if (node)
			{
				for (Engine::EntityID childID : node->Children)
				{
					Entity child{ childID, m_Context.get() };
					if (child) DrawEntityNode(child);
				}
			}
			ImGui::TreePop();
		}
	}
}

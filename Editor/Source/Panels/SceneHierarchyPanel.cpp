#include "SceneHierarchyPanel.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "Engine/Scene/Components.h"
#include "Engine/Utils/PrimitiveMeshFactory.h"

#include <ranges>

namespace Kairos
{
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		m_Context->EachEntity([&](Entity entity)
		{ DrawEntityNode({ entity, m_Context.get() }); });

		if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
			m_SelectionContext = {};

		ImGuiPopupFlags flags = ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight;

		if (ImGui::BeginPopupContextWindow(nullptr, flags))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
				m_Context->CreateEntity("Empty Entity");

			ImGui::Separator();

			// ---- Mesh Entities ----
			if (ImGui::BeginMenu("Mesh Entity"))
			{
				auto CreatePrimitive = [&](const char* name, const char* key)
				{
					Engine::Entity e = m_Context->CreateEntity(name);
					auto& mc = e.AddComponent<Engine::MeshComponent>();
					mc.SetPrimitiveMesh(key, Engine::PrimitiveMeshFactory::GetOrCreate(key));
					m_SelectionContext = e;
				};

				if (ImGui::MenuItem("Cube"))
					CreatePrimitive("Cube", Engine::PrimitiveKey::Cube);
				if (ImGui::MenuItem("Plane"))
					CreatePrimitive("Plane", Engine::PrimitiveKey::Plane);
				if (ImGui::MenuItem("Sphere"))
					CreatePrimitive("Sphere", Engine::PrimitiveKey::Sphere);

				ImGui::EndMenu();
			}

			// ---- Light Entities ----
			if (ImGui::BeginMenu("Light Entity"))
			{
				auto CreateLight = [&](const char* name, Engine::LightType type)
				{
					Engine::Entity e = m_Context->CreateEntity(name);
					auto& lc = e.AddComponent<Engine::LightComponent>();
					lc.Type = type;
					m_SelectionContext = e;
				};

				if (ImGui::MenuItem("Directional Light"))
					CreateLight("Directional Light", Engine::LightType::Directional);
				if (ImGui::MenuItem("Point Light"))
					CreateLight("Point Light", Engine::LightType::Point);
				if (ImGui::MenuItem("Spot Light"))
					CreateLight("Spot Light", Engine::LightType::Spot);

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
		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

		if (ImGui::IsItemClicked())
			m_SelectionContext = entity;

		bool entityDeleted = false;

		ImGuiPopupFlags popupFlags = ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight;

		if (ImGui::BeginPopupContextItem(nullptr, popupFlags))
		{
			if (ImGui::MenuItem("Delete"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (opened)
			ImGui::TreePop();

		if (entityDeleted)
		{
			if (m_SelectionContext == entity)
				m_SelectionContext = {};

			m_Context->DestroyEntity(entity);
		}
	}
}

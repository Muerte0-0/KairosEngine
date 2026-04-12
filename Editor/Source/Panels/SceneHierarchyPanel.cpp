#include "SceneHierarchyPanel.h"

#include "imgui.h"
#include "Engine/Scene/Components.h"

#include <glm/gtc/type_ptr.hpp>
#include <ranges>

#include "imgui_internal.h"

namespace Kairos
{
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");
		
		for (auto entity : std::views::reverse(m_Context->m_Registry.view<entt::entity>()))
			DrawEntityNode({ entity, m_Context.get() });
		
		if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
			m_SelectionContext = {};
		
		ImGuiPopupFlags flags = ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight;
		
		// Right-Click on a Blank Space
		if (ImGui::BeginPopupContextWindow(nullptr, flags))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
				m_Context->CreateEntity();
			
			ImGui::EndPopup();
		}
		
		ImGui::End();
		
		ImGui::Begin("Properties");
		
		if (m_SelectionContext)
			DrawComponents(m_SelectionContext);
		
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
	
	static void DrawVec3Control(const std::string& label, glm::vec3& values, float defaultValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGui::PushID(label.c_str());
		
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		
		ImGui::Text(label.c_str());
		ImGui::NextColumn();
		
		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0.0f, 0.0f});
		
		float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
		
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.4f, 0.1f, 0.15f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.6f, 0.2f, 0.2f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.4f, 0.1f, 0.15f, 1.0f});
		
		if (ImGui::Button("X", buttonSize))
			values.x = defaultValue;
		
		ImGui::PopStyleColor(3);
		
		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%0.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();
		
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.2f, 0.1f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.1f, 0.3f, 0.1f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.2f, 0.1f, 1.0f});
		
		if (ImGui::Button("Y", buttonSize))
			values.y = defaultValue;
		
		ImGui::PopStyleColor(3);
		
		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%0.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();
		
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.4f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.1f, 0.25f, 0.6f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.4f, 1.0f});
		
		if (ImGui::Button("Z", buttonSize))
			values.z = defaultValue;
		
		ImGui::PopStyleColor(3);
		
		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%0.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();
		
		ImGui::PopStyleVar();
		
		ImGui::Columns(1);
		
		ImGui::PopID();
	}
	
	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;
			
			char buffer[256] = {};
			strcpy_s(buffer, sizeof(buffer), tag.c_str());
			
			if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}
		
		ImGui::Separator();
		
		if (entity.HasComponent<TransformComponent>())
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
			
			if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), flags, "Transform"))
			{
				auto& tc = entity.GetComponent<TransformComponent>();
				DrawVec3Control("Translation", tc.Translation);
				glm::vec3 rotation = glm::degrees(tc.Rotation);
				DrawVec3Control("Rotation", rotation);
				tc.Rotation = glm::radians(rotation);
				DrawVec3Control("Scale", tc.Scale, 1.0f);
				
				ImGui::TreePop();
			}
			
		}
	}
}

#include "SceneHierarchyPanel.h"

#include "imgui.h"
#include "Engine/Scene/Components.h"

#include <glm/gtc/type_ptr.hpp>
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
		
		for (auto entity : std::views::reverse(m_Context->m_Registry.view<entt::entity>()))
			DrawEntityNode({ entity, m_Context.get() });
		
		if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
			m_SelectionContext = {};
		
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
		{
			m_SelectionContext = entity;
		}
		
		if (opened)
			ImGui::TreePop();
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
				ImGui::DragFloat3("Position", glm::value_ptr(tc.Translation), 0.1f);
				ImGui::DragFloat3("Rotation", glm::value_ptr(tc.Rotation), 0.1f);
				ImGui::DragFloat3("Scale", glm::value_ptr(tc.Scale), 0.1f);
				
				ImGui::TreePop();
			}
			
		}
	}
}

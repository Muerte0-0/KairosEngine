#include "SceneHierarchyPanel.h"

#include "imgui.h"
#include "Engine/Assets/MeshAssetManager.h"
#include "Engine/Scene/Components.h"

#include <algorithm>
#include <cstring>
#include <glm/gtc/type_ptr.hpp>
#include <ranges>

#include "imgui_internal.h"

namespace Kairos
{
	static bool DrawMeshAssetField(Engine::MeshComponent& meshComponent)
	{
		const std::string meshDisplayName = MeshAssetManager::GetDisplayName(meshComponent.MeshAssetPath);
		const float clearButtonWidth = 28.0f;
		const float fieldWidth = (std::max)(ImGui::CalcItemWidth() - clearButtonWidth - ImGui::GetStyle().ItemSpacing.x, 1.0f);

		ImGui::PushID("MeshAssetField");
		ImGui::Button(meshDisplayName.c_str(), ImVec2(fieldWidth, 0.0f));

		bool componentChanged = false;
		
		if (const ImGuiPayload* dragPayload = ImGui::GetDragDropPayload())
		{
			if (std::strcmp(dragPayload->DataType, "CONTENT_BROWSER_ITEM") == 0 && ImGui::IsItemHovered())
			{
				ImDrawList* drawList = ImGui::GetWindowDrawList();
				drawList->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
					ImGui::GetColorU32(ImGuiCol_DragDropTarget), 4.0f, 0, 2.0f);
			}
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* payloadPath = static_cast<const wchar_t*>(payload->Data);
				if (payloadPath != nullptr)
				{
					const std::filesystem::path meshAssetPath(payloadPath);
					
					if (!meshAssetPath.empty() && meshAssetPath != meshComponent.MeshAssetPath)
					{
						if (Ref<Mesh> mesh = MeshAssetManager::GetMesh(meshAssetPath))
							componentChanged = meshComponent.SetMeshAsset(meshAssetPath, mesh);
					}
				}
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::SameLine();
		if (ImGui::Button("X", ImVec2(clearButtonWidth, 0.0f)))
			componentChanged = meshComponent.ClearMesh() || componentChanged;

		ImGui::PopID();
		return componentChanged;
	}

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
		{
			DrawComponents(m_SelectionContext);
			
			if (ImGui::Button("Add Component"))
				ImGui::OpenPopup("AddComponent");
		
			if (ImGui::BeginPopup("AddComponent"))
			{
				if (ImGui::MenuItem("Camera Component"))
				{
					m_SelectionContext.AddComponent<CameraComponent>();
					ImGui::CloseCurrentPopup();
				}
			
				if (ImGui::MenuItem("Mesh Component"))
				{
					m_SelectionContext.AddComponent<MeshComponent>();
					ImGui::CloseCurrentPopup();
				}
			
				ImGui::EndPopup();
			}
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
		
		ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowOverlap;
		
		if (entity.HasComponent<TransformComponent>())
		{			
			if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), treeNodeFlags, "Transform"))
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
		
		ImGui::Separator();
		
		if (entity.HasComponent<MeshComponent>())
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4.0f, 4.0f});
			
			bool open = ImGui::TreeNodeEx((void*)typeid(MeshComponent).hash_code(), treeNodeFlags, "Mesh");
			
			ImGui::SameLine(ImGui::GetWindowWidth() - 25.f);
			if (ImGui::Button(":", ImVec2{20.0f, 20.0f}))
			{
				ImGui::OpenPopup("ComponentSettings");
			}
			ImGui::PopStyleVar();
			
			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove Component"))
					removeComponent = true;
				
				ImGui::EndPopup();
			}
			
			if (open)
			{
				auto& mc = entity.GetComponent<MeshComponent>();

				ImGui::Columns(2);
				ImGui::SetColumnWidth(0, 100.0f);
				ImGui::Text("Asset");
				ImGui::NextColumn();
				DrawMeshAssetField(mc);
				ImGui::Columns(1);
				
				ImGui::TreePop();
			}
			
			if (removeComponent)
				entity.RemoveComponent<MeshComponent>();
		}
		
		ImGui::Separator();
		
		if (entity.HasComponent<CameraComponent>())
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4.0f, 4.0f});
			
			bool open = ImGui::TreeNodeEx((void*)typeid(CameraComponent).hash_code(), treeNodeFlags, "Camera");
			
			ImGui::SameLine(ImGui::GetWindowWidth() - 25.f);
			if (ImGui::Button(":", ImVec2{20.0f, 20.0f}))
			{
				ImGui::OpenPopup("ComponentSettings");
			}
			ImGui::PopStyleVar();
			
			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove Component"))
					removeComponent = true;
				
				ImGui::EndPopup();
			}
			
			if (open)
			{
				//auto& mc = entity.GetComponent<CameraComponent>();
				
				ImGui::TreePop();
			}
			
			if (removeComponent)
				entity.RemoveComponent<CameraComponent>();
		}
	}
}

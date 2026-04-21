#include "PropertiesPanel.h"

#include "imgui.h"
#include "imgui_internal.h"
#include <glm/gtc/type_ptr.hpp>

#include "Engine/Assets/AssetManager.h"
#include "Engine/Assets/Editor/EditorAssetManager.h"
#include "Engine/Assets/Editor/AssetSerializer.h"
#include "Engine/Project/Project.h"
#include "Engine/Renderer/RHI/Resources/Mesh.h"
#include "Engine/Scene/Components.h"

#include <algorithm>
#include <cstring>

namespace Kairos
{
	static bool DrawMeshAssetField(Engine::MeshComponent& meshComponent)
	{
		using namespace Engine;

		std::string displayName = "None";
		if (meshComponent.HasMeshAsset())
		{
			auto editorAM = Project::GetActive()->GetEditorAssetManager();
			const AssetMetadata* meta = editorAM->GetRegistry().Get(meshComponent.MeshAssetHandle);
			if (meta) displayName = meta->FilePath.filename().string();
		}

		const float clearButtonWidth = 28.0f;
		const float fieldWidth = (std::max)(ImGui::CalcItemWidth() - clearButtonWidth - ImGui::GetStyle().ItemSpacing.x, 1.0f);

		ImGui::PushID("MeshAssetField");
		ImGui::Button(displayName.c_str(), ImVec2(fieldWidth, 0.0f));

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
				if (payloadPath)
				{
					std::filesystem::path droppedPath(payloadPath);
					auto editorAM = Project::GetActive()->GetEditorAssetManager();
					AssetHandle handle = editorAM->ImportAsset(droppedPath);
					if (static_cast<uint64_t>(handle) != NullAssetHandle && handle != meshComponent.MeshAssetHandle)
					{
						Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(handle);
						if (mesh)
							componentChanged = meshComponent.SetMeshAsset(handle, mesh, mesh->GetMaterials());
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

		ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4{0.4f, 0.1f, 0.15f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.6f, 0.2f, 0.2f,  1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4{0.4f, 0.1f, 0.15f, 1.0f});
		if (ImGui::Button("X", buttonSize)) values.x = defaultValue;
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%0.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4{0.1f, 0.2f, 0.1f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.1f, 0.3f, 0.1f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4{0.1f, 0.2f, 0.1f, 1.0f});
		if (ImGui::Button("Y", buttonSize)) values.y = defaultValue;
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%0.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4{0.1f, 0.25f, 0.4f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.1f, 0.25f, 0.6f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4{0.1f, 0.25f, 0.4f, 1.0f});
		if (ImGui::Button("Z", buttonSize)) values.z = defaultValue;
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%0.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
	}

	void PropertiesPanel::OnImGuiRender()
	{
		ImGui::Begin("Properties");

		if (m_SelectedAsset != Engine::AssetHandle(Engine::NullAssetHandle))
		{
			DrawAssetInspector(m_SelectedAsset);
		}
		else if (m_SelectionContext)
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

	static const char* AssetTypeLabel(Engine::AssetType type)
	{
		switch (type)
		{
			case Engine::AssetType::Mesh:     return "Mesh";
			case Engine::AssetType::Texture:  return "Texture";
			case Engine::AssetType::Material: return "Material";
			case Engine::AssetType::Shader:   return "Shader";
			case Engine::AssetType::Scene:    return "Scene";
			default:                          return "Unknown";
		}
	}

	void PropertiesPanel::DrawAssetInspector(Engine::AssetHandle handle)
	{
		using namespace Engine;

		auto editorAM = Project::GetActive()->GetEditorAssetManager();
		const AssetMetadata* meta = editorAM->GetRegistry().Get(handle);

		if (!meta || !meta->IsValid())
		{
			ImGui::TextDisabled("(no asset selected)");
			return;
		}

		ImGui::SeparatorText("Asset Inspector");

		ImGui::LabelText("Type",   "%s", AssetTypeLabel(meta->Type));
		ImGui::LabelText("Path",   "%s", meta->FilePath.string().c_str());
		ImGui::LabelText("Handle", "%llu", static_cast<uint64_t>(handle));
		ImGui::LabelText("Hash",   "%s", meta->SourceHash.empty() ? "(none)" : meta->SourceHash.c_str());

		bool stale = AssetSerializer::IsStale(*meta,
			Project::GetAssetDirectory() / meta->FilePath);
		if (stale)
			ImGui::TextColored({ 1.f, 0.6f, 0.1f, 1.f }, "  Source modified — reimport recommended");

		if (meta->Type == AssetType::Texture)
		{
			ImGui::Separator();
			ImGui::SeparatorText("Import Settings");

			AssetMetadata* mutable_meta = editorAM->GetRegistry().Get(handle);
			bool changed = false;
			changed |= ImGui::Checkbox("sRGB",          &mutable_meta->TextureSettings.sRGB);
			changed |= ImGui::Checkbox("Generate Mips", &mutable_meta->TextureSettings.GenerateMips);

			if (changed || ImGui::Button("Reimport", { -1.f, 0.f }))
				editorAM->ReimportAsset(handle);
		}
	}

	void PropertiesPanel::DrawComponents(Entity entity)
	{
		using namespace Engine;

		ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowOverlap;

		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;
			char buffer[256] = {};
			strcpy_s(buffer, sizeof(buffer), tag.c_str());
			if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
				tag = std::string(buffer);
		}

		ImGui::Separator();

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
				ImGui::OpenPopup("ComponentSettings");
			ImGui::PopStyleVar();

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove Component")) removeComponent = true;
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
				ImGui::OpenPopup("ComponentSettings");
			ImGui::PopStyleVar();

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove Component")) removeComponent = true;
				ImGui::EndPopup();
			}

			if (open)
				ImGui::TreePop();

			if (removeComponent)
				entity.RemoveComponent<CameraComponent>();
		}
	}
}

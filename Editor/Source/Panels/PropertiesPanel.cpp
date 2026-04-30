#include "PropertiesPanel.h"

#include "imgui.h"
#include "imgui_internal.h"
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>

#include "Engine/Assets/AssetManager.h"
#include "Engine/Assets/Editor/EditorAssetManager.h"
#include "Engine/Assets/Editor/AssetSerializer.h"
#include "Engine/Materials/MaterialGraph.h"
#include "Engine/Materials/MaterialGraphCompiler.h"
#include "Engine/Project/Project.h"
#include "Engine/Renderer/RHI/Resources/Mesh.h"
#include "Engine/Scene/Components.h"
#include "Engine/Utils/PrimitiveMeshFactory.h"

#include <algorithm>
#include <cstring>

namespace Kairos
{
	// Returns true if primitive selection changed
	static bool DrawPrimitivePicker(Engine::MeshComponent& mc)
	{
		using namespace Engine;

		const auto& registry = PrimitiveMeshFactory::GetRegistry();

		// Find current selection index (-1 = not a primitive / "None")
		int currentIdx = -1;
		for (int i = 0; i < static_cast<int>(registry.size()); ++i)
		{
			if (mc.PrimitiveKey == registry[i].Key)
			{
				currentIdx = i;
				break;
			}
		}

		// Build preview string
		const char* preview = (currentIdx >= 0) ? registry[currentIdx].DisplayName : "None";

		bool changed = false;
		ImGui::SetNextItemWidth(ImGui::CalcItemWidth());
		if (ImGui::BeginCombo("##PrimitivePicker", preview))
		{
			// "None" option — clears primitive (leaves mesh as-is for drag-drop)
			bool noneSelected = (currentIdx < 0);
			if (ImGui::Selectable("None", noneSelected) && !noneSelected)
			{
				mc.ClearMesh();
				changed = true;
			}

			for (int i = 0; i < static_cast<int>(registry.size()); ++i)
			{
				bool selected = (i == currentIdx);
				if (ImGui::Selectable(registry[i].DisplayName, selected) && !selected)
				{
					Ref<Mesh> mesh = PrimitiveMeshFactory::GetOrCreate(registry[i].Key);
					changed = mc.SetPrimitiveMesh(registry[i].Key, mesh);
				}
				if (selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		return changed;
	}

	static bool DrawMeshAssetField(Engine::MeshComponent& meshComponent)
	{
		using namespace Engine;

		// When a primitive is set, show its name; otherwise show asset filename or "None"
		std::string displayName = "None";
		if (meshComponent.IsPrimitive())
		{
			for (const auto& entry : PrimitiveMeshFactory::GetRegistry())
			{
				if (meshComponent.PrimitiveKey == entry.Key)
				{
					displayName = entry.DisplayName;
					break;
				}
			}
		}
		else if (meshComponent.HasMeshAsset())
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

	// Returns true if any slot changed (caller should mark scene dirty)
	static bool DrawMaterialSlots(Engine::MeshComponent& mc)
	{
		using namespace Engine;

		if (!mc.MeshRef) return false;

		const auto& subMeshes = mc.MeshRef->GetSubMeshes();
		if (subMeshes.empty()) return false;

		// Ensure Materials vector has enough entries
		if (mc.Materials.size() < subMeshes.size())
			mc.Materials.resize(subMeshes.size(), nullptr);

		bool changed = false;
		auto editorAM = Project::GetActive()->GetEditorAssetManager();

		ImGui::SeparatorText("Materials");

		for (uint32_t i = 0; i < static_cast<uint32_t>(subMeshes.size()); ++i)
		{
			ImGui::PushID(static_cast<int>(i));

			// Slot label
			std::string label = subMeshes[i].Name.empty()
				? ("Slot " + std::to_string(i))
				: subMeshes[i].Name;
			ImGui::Text("%s", label.c_str());
			ImGui::SameLine();

			// Display name: find .kmat in registry that matches this material
			std::string displayName = "Default";
			if (mc.Materials[i])
				displayName = mc.Materials[i]->GetName().empty() ? "Material" : mc.Materials[i]->GetName();

			float clearW = 24.f;
			float btnW   = ImGui::GetContentRegionAvail().x - clearW - ImGui::GetStyle().ItemSpacing.x;
			ImGui::Button(displayName.c_str(), ImVec2(btnW, 0));

			// Highlight drop target
			if (const ImGuiPayload* drag = ImGui::GetDragDropPayload())
			{
				if (std::strcmp(drag->DataType, "MATERIAL_ITEM") == 0 && ImGui::IsItemHovered())
				{
					ImGui::GetWindowDrawList()->AddRect(
						ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
						ImGui::GetColorU32(ImGuiCol_DragDropTarget), 4.f, 0, 2.f);
				}
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL_ITEM"))
				{
					std::filesystem::path droppedPath(static_cast<const wchar_t*>(payload->Data));
					AssetHandle handle = editorAM->ImportAsset(droppedPath);
					if (static_cast<uint64_t>(handle) != NullAssetHandle)
					{
						Ref<MaterialAsset> matAsset = AssetManager::GetAsset<MaterialAsset>(handle);
						if (matAsset)
						{
							if (matAsset->IsDirty || !matAsset->CompiledMaterial)
							{
								matAsset->CompiledMaterial = MaterialGraphCompiler::Compile(matAsset->Graph);
								matAsset->IsDirty = false;
							}
							mc.Materials[i] = matAsset->CompiledMaterial;
							changed = true;
						}
					}
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::SameLine();
			if (ImGui::Button("X", ImVec2(clearW, 0)))
			{
				mc.Materials[i] = nullptr;
				changed = true;
			}

			ImGui::PopID();
		}

		return changed;
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

			// Notify editor of any edit so it can mark the prefab dirty.
			if (OnEntityModified)
				OnEntityModified();

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

				if (ImGui::MenuItem("Light Component"))
				{
					m_SelectionContext.AddComponent<LightComponent>();
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
		
		if (entity.HasComponent<MeshComponent>())
		{
			ImGui::Separator();
			
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

				// Row 1: Primitive picker
				ImGui::Text("Primitive");
				ImGui::NextColumn();
				bool changed = DrawPrimitivePicker(mc);
				ImGui::NextColumn();

				// Row 2: Asset drag-drop (only relevant when no primitive selected)
				ImGui::Text("Asset");
				ImGui::NextColumn();
				changed |= DrawMeshAssetField(mc);
				ImGui::Columns(1);

				ImGui::Checkbox("Cast Shadows", &mc.CastShadows);

				DrawMaterialSlots(mc);

				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<MeshComponent>();
		}

		if (entity.HasComponent<CameraComponent>())
		{
			ImGui::Separator();
			
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
			{
				auto& cc = entity.GetComponent<CameraComponent>();
				auto& cam = cc.Camera;

				// Primary camera toggle — enforces single-primary via Scene
				bool primary = cc.Primary;
				if (ImGui::Checkbox("Primary Camera", &primary))
				{
					if (primary)
					{
						// Use Scene helper to clear all other primaries
						m_SelectionContext.GetScene()->SetPrimaryCamera(m_SelectionContext);
					}
					else
					{
						cc.Primary = false;
					}
				}

				ImGui::Separator();

				// FOV
				float fov = cam.GetFOV();
				if (ImGui::DragFloat("FOV", &fov, 0.5f, 1.0f, 179.0f, "%.1f deg"))
					cam.SetFOV(fov);

				// Near / Far
				float nearPlane = cam.GetNear();
				float farPlane  = cam.GetFar();
				if (ImGui::DragFloat("Near Plane", &nearPlane, 0.1f, 0.1f, 10.0f, "%.1f"))
					cam.SetNearFar(nearPlane, cam.GetFar());
				if (ImGui::DragFloat("Far Plane",  &farPlane,  1.0f,   0.1f,  10000.0f, "%.1f"))
					cam.SetNearFar(cam.GetNear(), farPlane);

				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<CameraComponent>();
		}

		if (entity.HasComponent<LightComponent>())
		{
			ImGui::Separator();
			
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4.0f, 4.0f});
			bool open = ImGui::TreeNodeEx((void*)typeid(LightComponent).hash_code(), treeNodeFlags, "Light");
			ImGui::SameLine(ImGui::GetWindowWidth() - 25.f);
			if (ImGui::Button(":", ImVec2{20.0f, 20.0f}))
				ImGui::OpenPopup("LightComponentSettings");
			ImGui::PopStyleVar();

			bool removeComponent = false;
			if (ImGui::BeginPopup("LightComponentSettings"))
			{
				if (ImGui::MenuItem("Remove Component")) removeComponent = true;
				ImGui::EndPopup();
			}

			if (open)
			{
				auto& lc = entity.GetComponent<LightComponent>();

				// Type selector
				const char* typeNames[] = { "Directional", "Point", "Spot" };
				int typeIdx = static_cast<int>(lc.Type);
				if (ImGui::Combo("Type", &typeIdx, typeNames, 3))
					lc.Type = static_cast<LightType>(typeIdx);

				ImGui::ColorEdit3("Color", &lc.Color.x);
				ImGui::DragFloat("Intensity", &lc.Intensity, 0.01f, 0.0f, 100.0f, "%.2f");

				ImGui::Separator();

				if (lc.Type == LightType::Directional)
				{
					ImGui::TextDisabled("Direction driven by entity rotation (-Y axis)");
				}
				else if (lc.Type == LightType::Point)
				{
					ImGui::DragFloat("Range", &lc.Point.Range, 0.1f, 0.01f, 1000.0f, "%.2f");
				}
				else if (lc.Type == LightType::Spot)
				{
					DrawVec3Control("Direction", lc.Spot.Direction);
					ImGui::DragFloat("Range", &lc.Spot.Range, 0.1f, 0.01f, 1000.0f, "%.2f");

					float innerDeg = glm::degrees(lc.Spot.InnerConeAngle);
					float outerDeg = glm::degrees(lc.Spot.OuterConeAngle);
					if (ImGui::DragFloat("Inner Cone", &innerDeg, 0.5f, 0.0f, 89.0f, "%.1f deg"))
						lc.Spot.InnerConeAngle = glm::radians(innerDeg);
					if (ImGui::DragFloat("Outer Cone", &outerDeg, 0.5f, 0.0f, 89.0f, "%.1f deg"))
						lc.Spot.OuterConeAngle = glm::radians(outerDeg);
					// Clamp inner <= outer
					lc.Spot.InnerConeAngle = std::min(lc.Spot.InnerConeAngle, lc.Spot.OuterConeAngle);
				}

				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<LightComponent>();
		}
	}
}

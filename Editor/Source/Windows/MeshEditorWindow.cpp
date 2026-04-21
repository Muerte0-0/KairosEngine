#include "MeshEditorWindow.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "Engine/Assets/AssetManager.h"
#include "Engine/Assets/Editor/EditorAssetManager.h"
#include "Engine/Project/Project.h"

namespace Kairos
{
	MeshEditorWindow::MeshEditorWindow(const std::filesystem::path& path, Engine::AssetHandle handle)
		: m_SourcePath(path), m_Handle(handle)
	{
		m_Title = path.stem().string();
		m_Mesh  = Engine::AssetManager::GetAsset<Mesh>(handle);
	}

	void MeshEditorWindow::OnImGuiRender()
	{
		if (!m_Open) return;

		if (m_OuterDockID != 0)
			ImGui::SetNextWindowDockID(m_OuterDockID, ImGuiCond_Appearing);

		std::string windowTitle = m_Title + "###MeshEditor_" + m_Title;

		if (!ImGui::Begin(windowTitle.c_str(), &m_Open, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return;
		}

		if (!m_Mesh)
		{
			ImGui::TextColored({ 1, 0.3f, 0.3f, 1 }, "Mesh failed to load.");
			ImGui::End();
			return;
		}

		const float totalWidth  = ImGui::GetContentRegionAvail().x;
		const float totalHeight = ImGui::GetContentRegionAvail().y;
		const float leftWidth   = totalWidth * 0.5f;
		const float rightWidth  = totalWidth - leftWidth - ImGui::GetStyle().ItemSpacing.x;

		// ── LEFT: Preview (orbit viewport deferred — no off-screen pass yet) ────
		ImGui::BeginChild("##MeshPreview", ImVec2(leftWidth, totalHeight), false);
		{
			ImGui::SeparatorText("Preview");

			// Placeholder until orbit camera + off-screen render pass is implemented
			ImVec2 avail = ImGui::GetContentRegionAvail();
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 1.f));
			ImGui::BeginChild("##MeshViewportPlaceholder", avail, false);
			{
				// Center text vertically
				float textY = avail.y * 0.5f - ImGui::GetTextLineHeight();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + textY);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
					(avail.x - ImGui::CalcTextSize("Viewport (coming soon)").x) * 0.5f);
				ImGui::TextDisabled("Viewport (coming soon)");
			}
			ImGui::EndChild();
			ImGui::PopStyleColor();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		// ── RIGHT ─────────────────────────────────────────────────────────────
		ImGui::BeginChild("##MeshRight", ImVec2(rightWidth, totalHeight), false);
		{
			const float halfH = totalHeight * 0.5f;

			// RIGHT-TOP: Asset Details
			ImGui::BeginChild("##MeshDetails", ImVec2(0.f, halfH - ImGui::GetStyle().ItemSpacing.y * 0.5f), true);
			{
				ImGui::SeparatorText("Asset Details");

				// Totals
				uint32_t totalVerts = (uint32_t)m_Mesh->GetVertices().size();
				uint32_t totalTris  = (uint32_t)m_Mesh->GetIndices().size() / 3u;
				ImGui::LabelText("File",       "%s", m_SourcePath.filename().string().c_str());
				ImGui::LabelText("Vertices",   "%u", totalVerts);
				ImGui::LabelText("Triangles",  "%u", totalTris);
				ImGui::LabelText("SubMeshes",  "%u", (uint32_t)m_Mesh->GetSubMeshes().size());

				// AABB
				auto aabb = m_Mesh->ComputeAABB();
				if (aabb.IsValid())
				{
					ImGui::Spacing();
					ImGui::SeparatorText("AABB (local)");
					ImGui::Text("Min: (%.3f, %.3f, %.3f)", aabb.Min.x, aabb.Min.y, aabb.Min.z);
					ImGui::Text("Max: (%.3f, %.3f, %.3f)", aabb.Max.x, aabb.Max.y, aabb.Max.z);
					glm::vec3 size = aabb.Max - aabb.Min;
					ImGui::Text("Size: (%.3f, %.3f, %.3f)", size.x, size.y, size.z);
				}

				// SubMesh table
				ImGui::Spacing();
				ImGui::SeparatorText("SubMeshes");

				if (ImGui::BeginTable("##SubMeshTable", 4,
					ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
					ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp,
					ImVec2(0.f, 0.f)))
				{
					ImGui::TableSetupScrollFreeze(0, 1);
					ImGui::TableSetupColumn("Name",     ImGuiTableColumnFlags_WidthStretch);
					ImGui::TableSetupColumn("Verts",    ImGuiTableColumnFlags_WidthFixed, 55.f);
					ImGui::TableSetupColumn("Tris",     ImGuiTableColumnFlags_WidthFixed, 55.f);
					ImGui::TableSetupColumn("Mat Slot", ImGuiTableColumnFlags_WidthFixed, 60.f);
					ImGui::TableHeadersRow();

					for (const auto& sm : m_Mesh->GetSubMeshes())
					{
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::TextUnformatted(sm.Name.empty() ? "(unnamed)" : sm.Name.c_str());
						ImGui::TableSetColumnIndex(1);
						ImGui::Text("%u", sm.IndexCount); // proxy — real vert count needs range
						ImGui::TableSetColumnIndex(2);
						ImGui::Text("%u", sm.IndexCount / 3u);
						ImGui::TableSetColumnIndex(3);
						ImGui::Text("%u", sm.MaterialIndex);
					}

					ImGui::EndTable();
				}
			}
			ImGui::EndChild();

			// RIGHT-BOTTOM: Import Settings
			ImGui::BeginChild("##MeshSettings", ImVec2(0.f, 0.f), true);
			{
				ImGui::SeparatorText("Import Settings");
				ImGui::TextDisabled("No import settings yet.");
				// TODO: scale, axis remapping, generate tangents toggle
				// when MeshImportSettings is added to .kasset
			}
			ImGui::EndChild();
		}
		ImGui::EndChild();

		ImGui::End();
	}
} // namespace Kairos

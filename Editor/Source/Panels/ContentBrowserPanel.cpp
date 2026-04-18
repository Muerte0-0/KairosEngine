#include "ContentBrowserPanel.h"

#include "Engine.h"

namespace Engine
{
	ContentBrowserPanel::ContentBrowserPanel() : m_BaseDirectory(Project::GetAssetDirectory()), m_CurrentDirectory(m_BaseDirectory)
	{
		m_DirectoryIcon = Texture::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon      = Texture::Create("Resources/Icons/ContentBrowser/FileIcon.png");
	}

	// Recursive folder tree for left panel
	void ContentBrowserPanel::RenderFolderTree(const std::filesystem::path& directory)
	{
		if (!std::filesystem::exists(directory)) return;

		for (auto& entry : std::filesystem::directory_iterator(directory))
		{
			if (!entry.is_directory()) continue;

			const std::string name = entry.path().filename().string();
			const bool isCurrent   = (entry.path() == m_CurrentDirectory);

			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
				| ImGuiTreeNodeFlags_SpanFullWidth;

			if (isCurrent)
				flags |= ImGuiTreeNodeFlags_Selected;

			// Check if this folder has sub-folders
			bool hasSubDirs = false;
			for (auto& sub : std::filesystem::directory_iterator(entry.path()))
			{
				if (sub.is_directory()) { hasSubDirs = true; break; }
			}
			if (!hasSubDirs)
				flags |= ImGuiTreeNodeFlags_Leaf;

			bool open = ImGui::TreeNodeEx(name.c_str(), flags);

			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				m_CurrentDirectory = entry.path();

			if (open)
			{
				RenderFolderTree(entry.path());
				ImGui::TreePop();
			}
		}
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		// ── Header: Back button + path breadcrumb ──────────────────────────────
		bool canGoBack = (m_CurrentDirectory != m_BaseDirectory);
		if (!canGoBack) ImGui::BeginDisabled();

		if (ImGui::ArrowButton("##Back", ImGuiDir_Left))
			m_CurrentDirectory = m_CurrentDirectory.parent_path();

		if (!canGoBack) ImGui::EndDisabled();

		ImGui::Separator();

		static float padding       = 8.f;
		static float thumbnailSize = 96.f;

		// ── Main layout: left folder tree | right icon grid ───────────────────
		// Table with 2 columns: fixed left pane, stretching right pane
		if (ImGui::BeginTable("##CBLayout", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable,
			{ 0, ImGui::GetContentRegionAvail().y - 36}))   // leave room for settings footer
		{
			ImGui::TableSetupColumn("##FolderTree", ImGuiTableColumnFlags_WidthFixed, 200.f);
			ImGui::TableSetupColumn("##IconGrid",   ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableNextRow();

			// ── Left: folder tree ──
			ImGui::TableSetColumnIndex(0);
			ImGui::BeginChild("##FolderTreeChild", ImVec2(0, 0), false);
			RenderFolderTree(m_BaseDirectory);
			ImGui::EndChild();

			// ── Right: icon grid ──
			ImGui::TableSetColumnIndex(1);
			
			// Build relative path from base for display
			std::string displayPath = std::filesystem::relative(m_CurrentDirectory, m_BaseDirectory.parent_path()).string();
			// Replace backslashes for consistency
			for (auto& c : displayPath) if (c == '\\') c = '/';
			
			ImGui::TextUnformatted(displayPath.c_str());
			
			ImGui::Separator();
			
			ImGui::BeginChild("##IconGridChild", ImVec2(0, 0), false);

			float cellSize    = thumbnailSize + padding;
			float panelWidth  = ImGui::GetContentRegionAvail().x;
			int   columnCount = std::max(1, static_cast<int>(panelWidth / cellSize));

			if (std::filesystem::exists(m_CurrentDirectory))
			{
				ImGui::Columns(columnCount, nullptr, false);

				for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
				{
					const auto& path = directoryEntry.path();
					std::string filenameString = path.filename().string();

					ImGui::PushID(filenameString.c_str());

					Ref<Texture> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;

					ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
					ImGui::ImageButton(filenameString.c_str(), icon->GetTextureID(), { thumbnailSize, thumbnailSize });

					if (ImGui::BeginDragDropSource())
					{
						const wchar_t* itemPath = path.c_str();
						ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);
						ImGui::TextUnformatted(filenameString.c_str());
						ImGui::EndDragDropSource();
					}

					ImGui::PopStyleColor();

					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						if (directoryEntry.is_directory())
							m_CurrentDirectory = path;
					}

					ImGui::TextUnformatted(filenameString.c_str());
					ImGui::NextColumn();
					ImGui::PopID();
				}
				
				ImGui::Columns(1);
				
				ImGui::Dummy({0, (ImGui::GetContentRegionAvail().y - 96)});
				
				// ── Settings ────────────────────────────────────────────────────
				ImGui::SeparatorText("Settings");
				ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16.f, 512.f);
				ImGui::SliderFloat("Padding",        &padding,       0.f,  32.f);
			}

			ImGui::EndChild(); // Icon Grid Child
			ImGui::EndTable();
		}
		ImGui::End();
	}
}

#include "ContentBrowserPanel.h"

#include "Engine.h"

namespace Engine
{
	ContentBrowserPanel::ContentBrowserPanel() : m_BaseDirectory(Project::GetAssetDirectory()), m_CurrentDirectory(m_BaseDirectory)
	{
		m_DirectoryIcon	= Texture::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon		= Texture::Create("Resources/Icons/ContentBrowser/FileIcon.png");
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		if (ImGui::ArrowButton("Back", ImGuiDir_Left) && m_CurrentDirectory != std::filesystem::path(m_BaseDirectory))
			m_CurrentDirectory = m_CurrentDirectory.parent_path();

		static float padding       = 8.f;
		static float thumbnailSize = 128.f;
		float cellSize = thumbnailSize + padding;

		float panelWidth  = ImGui::GetContentRegionAvail().x;
		int   columnCount = std::max(1, static_cast<int>(panelWidth / cellSize));

		ImGui::Columns(columnCount, nullptr, false);

		if (std::filesystem::exists(m_CurrentDirectory))
		{
			for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
			{
				const auto& path = directoryEntry.path();
				std::string filenameString = path.filename().string();
				
				ImGui::PushID(filenameString.c_str());

				Ref<Texture> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;

				ImGui::PushStyleColor(ImGuiCol_Button, {0});
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
						m_CurrentDirectory /= path.filename();
				}

				ImGui::Text("%s", filenameString.c_str());
				ImGui::NextColumn();
				
				ImGui::PopID();
			}
		}

		ImGui::Columns(1);
		
		ImGui::Dummy({0, (ImGui::GetContentRegionAvail().y - 96)});
		
		ImGui::SeparatorText("Settings");
		ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16.f, 512.f);
		ImGui::SliderFloat("Padding",        &padding,       0.f,  32.f);

		ImGui::End();
	}
}

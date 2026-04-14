#include "ContentBrowserPanel.h"

#include "Engine.h"

namespace Engine
{
	// To-Do: Change to Project Content Directory once Projects are set up
	extern const std::filesystem::path g_ContentDirectory = "Content";

	void ContentBrowserPanel::Init()
	{
		m_CurrentDirectory = g_ContentDirectory;

		// Create the Content directory if it doesn't exist yet
		if (!std::filesystem::exists(g_ContentDirectory))
			std::filesystem::create_directories(g_ContentDirectory);

		m_DirectoryIcon	= Texture::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon		= Texture::Create("Resources/Icons/ContentBrowser/FileIcon.png");
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		if (ImGui::ArrowButton("Back", ImGuiDir_Left) && m_CurrentDirectory != std::filesystem::path(g_ContentDirectory))
			m_CurrentDirectory = m_CurrentDirectory.parent_path();

		static float padding       = 8.f;
		static float thumbnailSize = 128.f;
		float cellSize = thumbnailSize + padding;

		float panelWidth  = ImGui::GetContentRegionAvail().x;
		int   columnCount = std::max(1, static_cast<int>(panelWidth / cellSize));

		ImGui::Columns(columnCount, nullptr, false);

		if (std::filesystem::exists(m_CurrentDirectory))
		{	
			int i = 0;
			
			for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
			{
				ImGui::PushID(i++);
				
				const auto& path           = directoryEntry.path();
				auto        relativePath   = std::filesystem::relative(path, g_ContentDirectory);
				std::string filenameString = relativePath.filename().string();

				Ref<Texture> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;

				ImGui::PushStyleColor(ImGuiCol_Button, {0});
				ImGui::ImageButton(filenameString.c_str(), icon->GetTextureID(), { thumbnailSize, thumbnailSize });
				
				if (ImGui::BeginDragDropSource())
				{
					const wchar_t* itemPath = relativePath.c_str();
					
					ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, wcslen(itemPath) * sizeof(wchar_t), ImGuiCond_Once);
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

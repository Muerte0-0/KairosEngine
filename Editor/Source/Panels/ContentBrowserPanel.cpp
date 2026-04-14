#include "ContentBrowserPanel.h"

#include "Engine.h"

namespace Engine
{
	// To-Do Change to Project Content Directory Once we have Projects Setup
	static const std::filesystem::path s_ContentDirectory = "Content";

	ContentBrowserPanel::ContentBrowserPanel() : m_CurrentDirectory(s_ContentDirectory)
	{
		
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");
		
		if (ImGui::Button("<-") && m_CurrentDirectory != std::filesystem::path(s_ContentDirectory))
			m_CurrentDirectory = m_CurrentDirectory.parent_path();
		
		static float padding = 8.f;
		static float thumbnailSize = 128;
		float cellSize = thumbnailSize + padding;
		
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = static_cast<int>(panelWidth / cellSize);
		
		ImGui::Columns(columnCount, 0, false);
		
		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, s_ContentDirectory);		
			std::string filenameString = relativePath.filename().string();
			
			ImGui::Button(filenameString.c_str(), { thumbnailSize, thumbnailSize });
			
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (directoryEntry.is_directory())
					m_CurrentDirectory /= path.filename();
			}
			
			ImGui::Text(filenameString.c_str());
			
			ImGui::NextColumn();
		}
		
		ImGui::Columns(1);
		
		ImGui::SeparatorText("Settings");
		
		ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
		ImGui::SliderFloat("Padding", &padding, 0, 32);
		
		ImGui::End();
	}
}

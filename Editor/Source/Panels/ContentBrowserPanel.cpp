#include "ContentBrowserPanel.h"

#include <filesystem>
#include <imgui.h>

namespace Engine
{
	// To-Do Change to Project Content Directory Once we have Projects Setup
	static const std::filesystem::path s_ContentDirectory = "Content";

	ContentBrowserPanel::ContentBrowserPanel() : m_CurrentDirectory(s_ContentDirectory) {}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");
		
		if (m_CurrentDirectory != std::filesystem::path(s_ContentDirectory))
		{
			if (ImGui::Button("<-"))
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
		}
		
		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, s_ContentDirectory);		
			std::string filenameString = relativePath.filename().string();
			
			if (directoryEntry.is_directory())
			{
				if (ImGui::Button(filenameString.c_str()))
				{
					m_CurrentDirectory /= path.filename();
				}
			}
			else
			{
				if (ImGui::Button(filenameString.c_str()))
				{
				}
			}
		}
		
		ImGui::End();
	}
}

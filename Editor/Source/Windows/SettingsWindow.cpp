#include "SettingsWindow.h"

#include <algorithm>

#include "Engine/Settings/SettingsRegistry.h"
#include "imgui.h"

namespace Kairos
{
	SettingsWindow::SettingsWindow(std::string title, bool editorOnly, std::filesystem::path configPath)
		: m_EditorOnly(editorOnly), m_ConfigPath(std::move(configPath))
	{
		m_Title = std::move(title);

		const auto& order = m_EditorOnly
			? Engine::SettingsRegistry::GetEditorSettingsOrder()
			: Engine::SettingsRegistry::GetProjectSettingsOrder();

		if (!order.empty())
			m_SelectedCategory = order.front();
	}

	void SettingsWindow::OnImGuiRender()
	{
		ImGuiCond dockCond = m_OuterDockID ? ImGuiCond_FirstUseEver : ImGuiCond_None;
		if (m_OuterDockID)
			ImGui::SetNextWindowDockID(m_OuterDockID, dockCond);

		bool open = m_Open;
		if (!ImGui::Begin(m_Title.c_str(), &open))
		{
			m_Open = open;
			ImGui::End();
			return;
		}
		m_Open = open;

		if (ImGui::Button("Save"))
			Engine::SettingsRegistry::SaveAll(m_ConfigPath);

		ImGui::Separator();

		const auto& order = m_EditorOnly
			? Engine::SettingsRegistry::GetEditorSettingsOrder()
			: Engine::SettingsRegistry::GetProjectSettingsOrder();

		const float leftWidth = 220.0f;
		ImGui::BeginChild("##SettingsCategories", ImVec2(leftWidth, 0.0f), true);
		DrawCategoryList(order);
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("##SettingsSections", ImVec2(0.0f, 0.0f), false);
		DrawSections(order);
		ImGui::EndChild();

		ImGui::End();
	}

	void SettingsWindow::DrawCategoryList(const std::vector<std::string>& order)
	{
		for (const std::string& category : order)
		{
			const bool selected = category == m_SelectedCategory;
			if (ImGui::Selectable(category.c_str(), selected))
				m_SelectedCategory = category;
		}
	}

	void SettingsWindow::DrawSections(const std::vector<std::string>& order)
	{
		if (m_SelectedCategory.empty() && !order.empty())
			m_SelectedCategory = order.front();

		Engine::GroupedSettings grouped = Engine::SettingsRegistry::BuildGrouped(m_EditorOnly);
		const auto& knownSections = Engine::SettingsRegistry::GetKnownSections();

		ImGui::TextUnformatted(m_SelectedCategory.c_str());
		ImGui::Separator();

		std::vector<std::string> sections;
		if (auto it = knownSections.find(m_SelectedCategory); it != knownSections.end())
			sections = it->second;

		for (const auto& [section, settings] : grouped[m_SelectedCategory])
		{
			if (std::find(sections.begin(), sections.end(), section) == sections.end())
				sections.push_back(section);
		}

		if (sections.empty())
			sections.push_back("No Sections");

		for (const std::string& sectionName : sections)
		{
			if (ImGui::TreeNode(sectionName.c_str()))
			{
				auto& list = grouped[m_SelectedCategory][sectionName];

				if (list.empty())
				{
					ImGui::TextDisabled("No settings available");
				}
				else
				{
					std::sort(list.begin(), list.end(), [](Engine::ISettings* lhs, Engine::ISettings* rhs)
					{
						return lhs->GetMetadata().Priority < rhs->GetMetadata().Priority;
					});

					for (Engine::ISettings* settings : list)
					{
						ImGui::PushID(settings);
						settings->DrawUI();
						ImGui::Separator();
						ImGui::PopID();
					}
				}

				ImGui::TreePop();
			}
		}
	}

	ProjectSettingsWindow::ProjectSettingsWindow(const std::filesystem::path& configPath)
		: SettingsWindow("Project Settings", false, configPath)
	{
	}

	EditorPreferencesWindow::EditorPreferencesWindow(const std::filesystem::path& configPath)
		: SettingsWindow("Editor Preferences", true, configPath)
	{
	}
}

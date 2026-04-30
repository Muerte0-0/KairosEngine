#pragma once

#include "EditorWindow.h"

#include <filesystem>
#include <string>
#include <vector>

namespace Kairos
{
	class SettingsWindow : public EditorWindow
	{
	public:
		SettingsWindow(std::string title, bool editorOnly, std::filesystem::path configPath);

		void OnImGuiRender() override;
		const std::string& GetTitle() const override { return m_Title; }

	private:
		void DrawCategoryList(const std::vector<std::string>& order);
		void DrawSections(const std::vector<std::string>& order);

		bool m_EditorOnly = false;
		std::filesystem::path m_ConfigPath;
		std::string m_SelectedCategory;
	};

	class ProjectSettingsWindow : public SettingsWindow
	{
	public:
		explicit ProjectSettingsWindow(const std::filesystem::path& configPath);
	};

	class EditorPreferencesWindow : public SettingsWindow
	{
	public:
		explicit EditorPreferencesWindow(const std::filesystem::path& configPath);
	};
}

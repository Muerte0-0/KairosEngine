#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include <yaml-cpp/yaml.h>

namespace Engine
{
	struct SettingsMetadata
	{
		std::string Category;
		std::string Section;
		int Priority = 0;
		bool IsEditorOnly = false;
	};

	class ISettings
	{
	public:
		virtual ~ISettings() = default;

		virtual const char* GetFileName() const = 0;
		virtual const char* GetTypeName() const = 0;

		virtual SettingsMetadata GetMetadata() const = 0;

		virtual void Serialize(YAML::Emitter& out) const = 0;
		virtual void Deserialize(const YAML::Node& node) = 0;

		virtual void DrawUI() = 0;
	};

	using SettingsSectionMap = std::unordered_map<std::string, std::vector<ISettings*>>;
	using GroupedSettings = std::unordered_map<std::string, SettingsSectionMap>;

	namespace SettingsCategories
	{
		inline const std::string Project        = "Project";
		inline const std::string Game           = "Game";
		inline const std::string Engine         = "Engine";
		inline const std::string Platforms      = "Platforms";
		inline const std::string Plugins        = "Plugins";

		inline const std::string General        = "General";
		inline const std::string LevelEditor    = "Level Editor";
		inline const std::string ContentEditors = "Content Editors";
		inline const std::string Privacy        = "Privacy";
		inline const std::string Advanced       = "Advanced";
	}

	class SettingsRegistry
	{
	public:
		static void Register(ISettings* settings);
		static const std::vector<ISettings*>& GetAll();

		static void RegisterDefaultSettings();

		static void LoadAll(const std::filesystem::path& configPath);
		static void SaveAll(const std::filesystem::path& configPath);

		static GroupedSettings BuildGrouped(bool editorOnly);
		static const std::unordered_map<std::string, std::vector<std::string>>& GetKnownSections();
		static const std::vector<std::string>& GetProjectSettingsOrder();
		static const std::vector<std::string>& GetEditorSettingsOrder();

	private:
		static const char* GetExtension(const SettingsMetadata& metadata);
		static void SaveOne(const std::filesystem::path& configPath, const ISettings& settings);
	};
}

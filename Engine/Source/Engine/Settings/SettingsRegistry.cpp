#include "kepch.h"
#include "SettingsRegistry.h"

#include <yaml-cpp/yaml.h>

#include "Engine/Settings/RenderSettings.h"

namespace Engine
{
	namespace
	{
		std::vector<ISettings*>& Storage()
		{
			static std::vector<ISettings*> settings;
			return settings;
		}

		std::vector<std::unique_ptr<ISettings>>& DefaultSettingsStorage()
		{
			static std::vector<std::unique_ptr<ISettings>> settings;
			return settings;
		}

		bool ContainsSetting(ISettings* settings)
		{
			const auto& all = Storage();
			return std::find(all.begin(), all.end(), settings) != all.end();
		}
	}

	void SettingsRegistry::Register(ISettings* settings)
	{
		if (!settings || ContainsSetting(settings))
			return;

		Storage().push_back(settings);
	}

	const std::vector<ISettings*>& SettingsRegistry::GetAll()
	{
		return Storage();
	}

	void SettingsRegistry::RegisterDefaultSettings()
	{
		auto& defaults = DefaultSettingsStorage();
		if (!defaults.empty())
			return;

		defaults.push_back(std::make_unique<RenderSettings>());

		for (const auto& settings : defaults)
			Register(settings.get());
	}

	void SettingsRegistry::LoadAll(const std::filesystem::path& configPath)
	{
		RegisterDefaultSettings();
		std::filesystem::create_directories(configPath);

		for (ISettings* settings : Storage())
		{
			const SettingsMetadata metadata = settings->GetMetadata();
			const std::filesystem::path path = configPath / (std::string(settings->GetFileName()) + GetExtension(metadata));

			if (!std::filesystem::exists(path))
			{
				SaveOne(configPath, *settings);
				continue;
			}

			YAML::Node root;
			try
			{
				root = YAML::LoadFile(path.string());
			}
			catch (const YAML::Exception& e)
			{
				LOG(LogLevel::Error, "SettingsRegistry: failed to parse '{}': {}", path.string(), e.what());
				continue;
			}

			if (!root["Type"] || root["Type"].as<std::string>() != settings->GetTypeName())
			{
				LOG(LogLevel::Warning, "SettingsRegistry: skipped '{}' because Type did not match '{}'.", path.string(), settings->GetTypeName());
				continue;
			}

			try
			{
				settings->Deserialize(root);
			}
			catch (const YAML::Exception& e)
			{
				LOG(LogLevel::Error, "SettingsRegistry: failed to load '{}': {}", path.string(), e.what());
			}
		}
	}

	void SettingsRegistry::SaveAll(const std::filesystem::path& configPath)
	{
		RegisterDefaultSettings();
		std::filesystem::create_directories(configPath);

		for (const ISettings* settings : Storage())
			SaveOne(configPath, *settings);
	}

	GroupedSettings SettingsRegistry::BuildGrouped(bool editorOnly)
	{
		GroupedSettings grouped;

		for (ISettings* settings : Storage())
		{
			const SettingsMetadata metadata = settings->GetMetadata();
			if (metadata.IsEditorOnly != editorOnly)
				continue;

			grouped[metadata.Category][metadata.Section].push_back(settings);
		}

		return grouped;
	}

	const std::unordered_map<std::string, std::vector<std::string>>& SettingsRegistry::GetKnownSections()
	{
		static const std::unordered_map<std::string, std::vector<std::string>> knownSections =
		{
			{ "Project", { "Description", "Supported Platforms" } },
			{ "Game", { "Asset Manager" } },
			{ "Engine", { "Audio", "Physics", "Input", "Network", "Rendering", "User Interface" } },
			{ "Platforms", { "Android", "Windows", "Linux" } },
			{ "Plugins", { } },

			{ "General", { "Appearance", "Global", "Source Code" } },
			{ "Level Editor", { "Miscellaneous", "Play", "Viewports" } },
			{ "Content Editors", { "Content Browser", "Mesh Editor", "Texture Editor" } },
			{ "Privacy", { "Bug Reports", "Usage Data" } },
			{ "Advanced", { } }
		};

		return knownSections;
	}

	const std::vector<std::string>& SettingsRegistry::GetProjectSettingsOrder()
	{
		static const std::vector<std::string> order =
		{
			"Project", "Game", "Engine", "Platforms", "Plugins"
		};

		return order;
	}

	const std::vector<std::string>& SettingsRegistry::GetEditorSettingsOrder()
	{
		static const std::vector<std::string> order =
		{
			"General", "Level Editor", "Content Editors", "Privacy", "Plugins", "Advanced"
		};

		return order;
	}

	const char* SettingsRegistry::GetExtension(const SettingsMetadata& metadata)
	{
		return metadata.IsEditorOnly ? ".kprefs" : ".kcfg";
	}

	void SettingsRegistry::SaveOne(const std::filesystem::path& configPath, const ISettings& settings)
	{
		const SettingsMetadata metadata = settings.GetMetadata();
		const std::filesystem::path path = configPath / (std::string(settings.GetFileName()) + GetExtension(metadata));

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Type" << YAML::Value << settings.GetTypeName();
		out << YAML::Key << "Version" << YAML::Value << 1;
		settings.Serialize(out);
		out << YAML::EndMap;

		std::ofstream fout(path);
		if (!fout)
		{
			LOG(LogLevel::Error, "SettingsRegistry: failed to write '{}'.", path.string());
			return;
		}

		fout << out.c_str();
	}
}

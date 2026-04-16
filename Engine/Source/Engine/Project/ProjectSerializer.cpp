#include "kepch.h"
#include "ProjectSerializer.h"

#include <yaml-cpp/yaml.h>

namespace Engine
{
	ProjectSerializer::ProjectSerializer(Ref<Project> project) : m_Project(project) {}

	bool ProjectSerializer::Serialize(const std::filesystem::path& filepath) const
	{
		const auto& config = m_Project->GetConfig();
		
		YAML::Emitter out;
		
		out << YAML::BeginMap; // Root
		{
			out << YAML::Key << "Project" << YAML::Value;
		
			out << YAML::BeginMap; // Project
			{
				out << YAML::Key << "Name" << YAML::Value << config.Name;
				out << YAML::Key << "StartupScene" << YAML::Value << config.StartupScene.string();
				out << YAML::Key << "AssetDirectory" << YAML::Value << config.AssetDirectory.string();
				out << YAML::Key << "AssetRegistry" << YAML::Value << config.AssetRegistryPath.string();
				out << YAML::Key << "SourceDirectory" << YAML::Value << config.SourceDirectory.string();
				
				out << YAML::EndMap;
			}
			
			out << YAML::EndMap;
		}
		
		std::ofstream fout(filepath);
		fout << out.c_str();
		
		return true;
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& filepath)
	{
		auto& config = m_Project->GetConfig();
		
		YAML::Node data;

		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException e)
		{
			LOG(LogLevel::Error, "Failed to Load Project: '{0}'\n {1}", filepath.string(), e.what());
			return false;
		}
		
		auto projectNode = data["Project"];
		
		if (!projectNode)
			return false;
		
		config.Name = projectNode["Name"].as<std::string>();
		config.StartupScene = projectNode["StartupScene"].as<std::string>();
		config.AssetDirectory = projectNode["AssetDirectory"].as<std::string>();
		config.AssetRegistryPath = projectNode["AssetRegistry"].as<std::string>();
		config.SourceDirectory = projectNode["SourceDirectory"].as<std::string>();
		
		return true;
	}
}

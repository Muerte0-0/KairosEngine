#pragma once

#include <string>
#include <filesystem>

namespace Engine
{
	struct ProjectSettings
	{
		std::string Name = "Untitled";
		
		std::filesystem::path AssetDirectory;
		std::filesystem::path AssetRegistryPath;
		
		std::filesystem::path SourceDirectory;
		
		std::string StartupScene;
		
		// Not Serialized
		std::string ProjectFileName;
		std::string ProjectDirectory;
	};
	
	class Project
	{
	public:
		
	};
}

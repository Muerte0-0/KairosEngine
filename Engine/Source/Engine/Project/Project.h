#pragma once

#include <string>
#include <filesystem>
#include "Engine/Assets/AssetManagerBase.h"
#include "Engine/Assets/Editor/EditorAssetManager.h"
#include "Engine/Assets/Runtime/RuntimeAssetManager.h"

namespace Engine
{
	struct ProjectConfig
	{
		std::string Name = "Untitled";
		
		std::filesystem::path StartupScene;
		
		std::filesystem::path AssetDirectory;
		std::filesystem::path AssetRegistryPath;
		
		std::filesystem::path SourceDirectory;
		
		// Not Serialized
		std::string ProjectFileName;
		std::string ProjectDirectory;
	};
	
	class Project
	{
	public:
		Project() = default;
		
		static std::filesystem::path GetProjectDirectory() { ASSERT(s_ActiveProject, "No Active Project Found") return s_ActiveProject->m_ProjectDirectory; }
		static std::filesystem::path GetAssetDirectory() { ASSERT(s_ActiveProject, "No Active Project Found") return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory; }
		
		// To-Do: Move to Asset Manager
		static std::filesystem::path GetAssetPath(const std::filesystem::path& path)
		{
			ASSERT(s_ActiveProject, "No Active Project Found")
			return GetAssetDirectory() / path;
		}
		
		ProjectConfig& GetConfig() { return m_Config; }

		std::shared_ptr<AssetManagerBase>		GetAssetManager()			{ return m_AssetManager; }
		std::shared_ptr<EditorAssetManager>		GetEditorAssetManager()		{ return std::static_pointer_cast<EditorAssetManager>(m_AssetManager); }
		std::shared_ptr<RuntimeAssetManager>	GetRuntimeAssetManager()	{ return std::static_pointer_cast<RuntimeAssetManager>(m_AssetManager); }

		void SetAssetManager(Ref<AssetManagerBase> manager) { m_AssetManager = std::move(manager); }
		
		static Ref<Project> GetActive() { return s_ActiveProject; }
		static void         SetActive(Ref<Project> project) { s_ActiveProject = std::move(project); }
		
		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& projectPath);
		static bool Save(const std::filesystem::path& projectPath);
		
	private:
		ProjectConfig m_Config;
		std::filesystem::path m_ProjectDirectory;
		Ref<AssetManagerBase> m_AssetManager;

		inline static Ref<Project> s_ActiveProject;
	};
}

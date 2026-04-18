#include "kepch.h"
#include "EditorAssetManager.h"
#include "AssetImporter.h"
#include "Engine/Project/Project.h"

#include <yaml-cpp/yaml.h>

namespace Engine
{
	// -----------------------------------------------------------------------
	// Helpers
	// -----------------------------------------------------------------------
	static std::string AssetTypeToString(AssetType type)
	{
		switch (type)
		{
			case AssetType::Mesh:     return "Mesh";
			case AssetType::Texture:  return "Texture";
			case AssetType::Material: return "Material";
			case AssetType::Scene:    return "Scene";
			default:                  return "None";
		}
	}

	static AssetType AssetTypeFromString(const std::string& str)
	{
		if (str == "Mesh")     return AssetType::Mesh;
		if (str == "Texture")  return AssetType::Texture;
		if (str == "Material") return AssetType::Material;
		if (str == "Scene")    return AssetType::Scene;
		return AssetType::None;
	}

	// -----------------------------------------------------------------------
	// EditorAssetManager
	// -----------------------------------------------------------------------
	EditorAssetManager::EditorAssetManager()
	{
		LoadRegistry();
	}

	void EditorAssetManager::LoadRegistry()
	{
		auto& config       = Project::GetActive()->GetConfig();
		std::filesystem::path registryPath = Project::GetProjectDirectory() / config.AssetRegistryPath;

		if (!std::filesystem::exists(registryPath))
		{
			LOG(LogLevel::Info, "EditorAssetManager: no registry found at '{}', starting fresh.", registryPath.string());
			return;
		}

		YAML::Node root;
		try   { root = YAML::LoadFile(registryPath.string()); }
		catch (const YAML::Exception& e)
		{
			LOG(LogLevel::Error, "EditorAssetManager: failed to parse registry: {}", e.what());
			return;
		}

		auto assets = root["Assets"];
		if (!assets) return;

		for (auto entry : assets)
		{
			AssetMetadata metadata;
			metadata.Handle   = AssetHandle(entry["Handle"].as<uint64_t>());
			metadata.Type     = AssetTypeFromString(entry["Type"].as<std::string>());
			metadata.FilePath = entry["Path"].as<std::string>();

			if (!metadata.IsValid())
				continue;

			m_Registry.Add(metadata);
		}

		LOG(LogLevel::Info, "EditorAssetManager: loaded {} asset(s) from registry.", m_Registry.Count());
	}

	void EditorAssetManager::SaveRegistry() const
	{
		auto& config       = Project::GetActive()->GetConfig();
		std::filesystem::path registryPath = Project::GetProjectDirectory() / config.AssetRegistryPath;

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;

		for (const auto& [handle, metadata] : m_Registry)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Handle" << YAML::Value << static_cast<uint64_t>(metadata.Handle);
			out << YAML::Key << "Type"   << YAML::Value << AssetTypeToString(metadata.Type);
			out << YAML::Key << "Path"   << YAML::Value << metadata.FilePath.generic_string();
			out << YAML::EndMap;
		}

		out << YAML::EndSeq << YAML::EndMap;

		std::ofstream file(registryPath);
		file << out.c_str();
	}

	AssetHandle EditorAssetManager::ImportAsset(const std::filesystem::path& rawPath)
	{
		// Normalize: always store relative to asset directory
		std::filesystem::path canonical = std::filesystem::weakly_canonical(rawPath);
		std::filesystem::path relative  = std::filesystem::relative(canonical, Project::GetAssetDirectory());

		// Already imported?
		if (m_Registry.IsPathRegistered(relative))
			return m_Registry.GetHandleForPath(relative);

		AssetType type = AssetImporter::DeduceTypeFromPath(relative);
		if (type == AssetType::None)
		{
			LOG(LogLevel::Warning, "EditorAssetManager: unsupported file type: '{}'.", relative.string());
			return AssetHandle(NullAssetHandle);
		}

		AssetMetadata metadata;
		metadata.Handle   = AssetHandle(); // generate new UUID
		metadata.Type     = type;
		metadata.FilePath = relative;

		m_Registry.Add(metadata);
		SaveRegistry();

		LOG(LogLevel::Info, "EditorAssetManager: imported '{}' as handle {}.",
			relative.string(), static_cast<uint64_t>(metadata.Handle));

		return metadata.Handle;
	}

	// -----------------------------------------------------------------------
	// AssetManagerBase impl
	// -----------------------------------------------------------------------
	Ref<Asset> EditorAssetManager::GetAsset(AssetHandle handle)
	{
		// Return cached
		auto it = m_LoadedAssets.find(handle);
		if (it != m_LoadedAssets.end())
			return it->second;

		// Not loaded — check registry
		const AssetMetadata* metadata = m_Registry.Get(handle);
		if (!metadata || !metadata->IsValid())
		{
			LOG(LogLevel::Warning, "EditorAssetManager: handle {} not in registry.", static_cast<uint64_t>(handle));
			return nullptr;
		}

		Ref<Asset> asset = LoadAsset(*metadata);
		if (asset)
			m_LoadedAssets[handle] = asset;

		return asset;
	}

	Ref<Asset> EditorAssetManager::LoadAsset(const AssetMetadata& metadata)
	{
		return AssetImporter::Import(metadata);
	}

	bool EditorAssetManager::IsAssetLoaded(AssetHandle handle) const
	{
		return m_LoadedAssets.contains(handle);
	}

	bool EditorAssetManager::IsAssetValid(AssetHandle handle) const
	{
		const AssetMetadata* metadata = m_Registry.Get(handle);
		return metadata && metadata->IsValid();
	}

	AssetType EditorAssetManager::GetAssetType(AssetHandle handle) const
	{
		const AssetMetadata* metadata = m_Registry.Get(handle);
		return metadata ? metadata->Type : AssetType::None;
	}
}

#include "kepch.h"
#include "EditorAssetManager.h"
#include "AssetImporter.h"
#include "KassetSerializer.h"
#include "Engine/Project/Project.h"

namespace Engine
{
	EditorAssetManager::EditorAssetManager()
	{
		ScanAndValidateAssets(Project::GetAssetDirectory());
	}

	// -----------------------------------------------------------------------
	// ScanAndValidateAssets
	//
	// Two passes over the asset directory:
	//   Pass 1 — collect all .kasset files, load into registry.
	//   Pass 2 — walk source files; if a supported file has no .kasset,
	//            create one (auto-assign handle + type).
	// -----------------------------------------------------------------------
	void EditorAssetManager::ScanAndValidateAssets(const std::filesystem::path& directory)
	{
		if (!std::filesystem::exists(directory))
			return;

		const std::filesystem::path& assetDir = Project::GetAssetDirectory();

		// ------------------------------------------------------------------
		// Pass 1: load existing .kasset files
		// ------------------------------------------------------------------
		size_t loaded = 0;
		for (const auto& entry : std::filesystem::recursive_directory_iterator(directory))
		{
			if (!entry.is_regular_file()) continue;
			if (entry.path().extension() != ".kasset") continue;

			AssetMetadata metadata = KassetSerializer::Read(entry.path());
			if (!metadata.IsValid())
			{
				LOG(LogLevel::Warning, "EditorAssetManager: invalid .kasset skipped: '{}'.", entry.path().string());
				continue;
			}

			// Derive source path: strip ".kasset" suffix
			std::filesystem::path sourcePath = entry.path().parent_path() / entry.path().stem();
			std::filesystem::path canonical  = std::filesystem::weakly_canonical(sourcePath);
			std::filesystem::path relative   = std::filesystem::relative(canonical, assetDir);

			metadata.FilePath = relative;

			if (m_Registry.Contains(metadata.Handle))
			{
				LOG(LogLevel::Warning, "EditorAssetManager: duplicate handle {} in '{}', skipped.",
					static_cast<uint64_t>(metadata.Handle), entry.path().string());
				continue;
			}

			m_Registry.Add(metadata);
			m_HandleToSourcePath[metadata.Handle] = canonical;
			++loaded;
		}

		LOG(LogLevel::Info, "EditorAssetManager: loaded {} Asset(s).", loaded);

		// ------------------------------------------------------------------
		// find source files with no Metadata, create the Metadata
		// ------------------------------------------------------------------
		size_t created = 0;
		for (const auto& entry : std::filesystem::recursive_directory_iterator(directory))
		{
			if (!entry.is_regular_file()) continue;

			// Skip .kasset files themselves
			if (entry.path().extension() == ".kasset") continue;

			AssetType type = AssetImporter::DeduceTypeFromPath(entry.path());
			if (type == AssetType::None) continue;

			std::filesystem::path kassetPath = KassetSerializer::GetKassetPath(entry.path());
			if (std::filesystem::exists(kassetPath)) continue; // already has sidecar

			// Build metadata for the orphaned source file
			std::filesystem::path canonical = std::filesystem::weakly_canonical(entry.path());
			std::filesystem::path relative  = std::filesystem::relative(canonical, assetDir);

			AssetMetadata metadata;
			metadata.Handle   = AssetHandle(); // new UUID
			metadata.Type     = type;
			metadata.FilePath = relative;

			if (!KassetSerializer::Write(canonical, metadata))
			{
				LOG(LogLevel::Error, "EditorAssetManager: failed to create Metadata for '{}'.", relative.string());
				continue;
			}

			// Re-read so SourceHash is populated from what was written
			AssetMetadata written = KassetSerializer::Read(kassetPath);
			written.FilePath      = relative;

			if (m_Registry.Contains(written.Handle))
				continue; // UUID collision (astronomically unlikely)

			m_Registry.Add(written);
			m_HandleToSourcePath[written.Handle] = canonical;
			++created;

			LOG(LogLevel::Info, "EditorAssetManager: Created Metadata for '{}' (handle {}).",
				relative.string(), static_cast<uint64_t>(written.Handle));
		}

		if (created > 0)
			LOG(LogLevel::Info, "EditorAssetManager: Auto-Created {} Metadata file(s).", created);
	}

	// -----------------------------------------------------------------------
	// ImportAsset
	// -----------------------------------------------------------------------
	AssetHandle EditorAssetManager::ImportAsset(const std::filesystem::path& rawPath)
	{
		std::filesystem::path canonical = std::filesystem::weakly_canonical(rawPath);
		std::filesystem::path relative  = std::filesystem::relative(canonical, Project::GetAssetDirectory());

		if (m_Registry.IsPathRegistered(relative))
			return m_Registry.GetHandleForPath(relative);

		AssetType type = AssetImporter::DeduceTypeFromPath(relative);
		if (type == AssetType::None)
		{
			LOG(LogLevel::Warning, "EditorAssetManager: unsupported file type: '{}'.", relative.string());
			return AssetHandle(NullAssetHandle);
		}

		AssetMetadata metadata;
		metadata.Handle   = AssetHandle();
		metadata.Type     = type;
		metadata.FilePath = relative;

		if (!KassetSerializer::Write(canonical, metadata))
			return AssetHandle(NullAssetHandle);

		std::filesystem::path kassetPath = KassetSerializer::GetKassetPath(canonical);
		AssetMetadata written = KassetSerializer::Read(kassetPath);
		written.FilePath      = relative;

		m_Registry.Add(written);
		m_HandleToSourcePath[written.Handle] = canonical;

		LOG(LogLevel::Info, "EditorAssetManager: imported '{}' as handle {}.",
			relative.string(), static_cast<uint64_t>(written.Handle));

		return written.Handle;
	}

	// -----------------------------------------------------------------------
	// ReimportAsset
	// -----------------------------------------------------------------------
	void EditorAssetManager::ReimportAsset(AssetHandle handle)
	{
		const AssetMetadata* metadata = m_Registry.Get(handle);
		if (!metadata)
		{
			LOG(LogLevel::Warning, "EditorAssetManager::ReimportAsset — handle {} not in registry.",
				static_cast<uint64_t>(handle));
			return;
		}

		auto it = m_HandleToSourcePath.find(handle);
		if (it == m_HandleToSourcePath.end())
		{
			LOG(LogLevel::Warning, "EditorAssetManager::ReimportAsset — no source path for handle {}.",
				static_cast<uint64_t>(handle));
			return;
		}

		// Rewrite .kasset (refreshes SourceHash)
		KassetSerializer::Write(it->second, *metadata);

		// Evict loaded asset — next GetAsset() call will re-run importer
		m_LoadedAssets.erase(handle);

		LOG(LogLevel::Info, "EditorAssetManager: reimport queued for handle {}.", static_cast<uint64_t>(handle));
	}

	// -----------------------------------------------------------------------
	// AssetManagerBase impl
	// -----------------------------------------------------------------------
	Ref<Asset> EditorAssetManager::GetAsset(AssetHandle handle)
	{
		auto it = m_LoadedAssets.find(handle);
		if (it != m_LoadedAssets.end())
			return it->second;

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

	bool EditorAssetManager::IsAssetLoaded(AssetHandle handle) const  { return m_LoadedAssets.contains(handle); }

	bool EditorAssetManager::IsAssetValid(AssetHandle handle) const
	{
		const AssetMetadata* m = m_Registry.Get(handle);
		return m && m->IsValid();
	}

	AssetType EditorAssetManager::GetAssetType(AssetHandle handle) const
	{
		const AssetMetadata* m = m_Registry.Get(handle);
		return m ? m->Type : AssetType::None;
	}

} // namespace Engine

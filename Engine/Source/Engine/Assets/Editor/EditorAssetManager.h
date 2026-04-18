#pragma once
#include "Engine/Assets/AssetManagerBase.h"
#include "Engine/Assets/AssetRegistry.h"

namespace Engine
{
	class EditorAssetManager : public AssetManagerBase
	{
	public:
		EditorAssetManager();
		~EditorAssetManager() override = default;

		// Scan directory for .kasset files and populate in-memory registry.
		// Also walks for source files missing a .kasset and creates them.
		// Called automatically on construction.
		void ScanAndValidateAssets(const std::filesystem::path& directory);

		// Import raw source file -> write .kasset sidecar, register in memory.
		// Returns existing handle if already imported.
		AssetHandle ImportAsset(const std::filesystem::path& rawPath);

		// Re-run importer for handle. Preserves handle. Evicts loaded cache.
		void ReimportAsset(AssetHandle handle);

		// AssetManagerBase
		Ref<Asset> GetAsset(AssetHandle handle) override;
		bool       IsAssetLoaded(AssetHandle handle) const override;
		bool       IsAssetValid(AssetHandle handle)  const override;
		AssetType  GetAssetType(AssetHandle handle)  const override;

		const AssetRegistry& GetRegistry() const { return m_Registry; }

	private:
		Ref<Asset> LoadAsset(const AssetMetadata& metadata);

		AssetRegistry                                          m_Registry;
		std::unordered_map<AssetHandle, Ref<Asset>>            m_LoadedAssets;
		// Absolute source path per handle — needed for reimport / hash check
		std::unordered_map<AssetHandle, std::filesystem::path> m_HandleToSourcePath;
	};
}

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

		// Registry persistence
		void LoadRegistry();
		void SaveRegistry() const;

		// Import a raw file → assigns UUID, normalizes path, adds to registry.
		// Returns existing handle if already imported.
		AssetHandle ImportAsset(const std::filesystem::path& rawPath);

		// Walk the asset directory and register any files not yet in the registry.
		// Called automatically on construction after LoadRegistry().
		void ScanAndRegisterDirectory(const std::filesystem::path& directory);

		// AssetManagerBase
		Ref<Asset> GetAsset(AssetHandle handle) override;
		bool       IsAssetLoaded(AssetHandle handle) const override;
		bool       IsAssetValid(AssetHandle handle)  const override;
		AssetType  GetAssetType(AssetHandle handle)  const override;

		const AssetRegistry& GetRegistry() const { return m_Registry; }

	private:
		Ref<Asset> LoadAsset(const AssetMetadata& metadata);

		AssetRegistry                               m_Registry;
		std::unordered_map<AssetHandle, Ref<Asset>> m_LoadedAssets;
	};
}

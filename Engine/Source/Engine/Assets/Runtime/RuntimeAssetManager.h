#pragma once
#include "Engine/Assets/AssetManagerBase.h"

namespace Engine
{
	// Minimal runtime stub — loads pre-cooked asset packs (future).
	// Never depends on AssetMetadata, AssetRegistry, or filesystem paths.
	class RuntimeAssetManager : public AssetManagerBase
	{
	public:
		RuntimeAssetManager()  = default;
		~RuntimeAssetManager() override = default;

		// Future: load a cooked .kpack binary blob
		// void LoadAssetPack(const std::filesystem::path& packPath);

		// AssetManagerBase
		Ref<Asset> GetAsset(AssetHandle handle) override;
		bool       IsAssetLoaded(AssetHandle handle) const override;
		bool       IsAssetValid(AssetHandle handle)  const override;
		AssetType  GetAssetType(AssetHandle handle)  const override;

	private:
		std::unordered_map<AssetHandle, Ref<Asset>> m_Assets;
	};
}

#pragma once
#include "Engine/Assets/AssetMetadata.h"
#include "Engine/Assets/Asset.h"

namespace Engine
{
	class AssetImporter
	{
	public:
		AssetImporter() = delete;

		// Deduce AssetType from file extension. Returns AssetType::None if unsupported.
		static AssetType DeduceTypeFromPath(const std::filesystem::path& path);

		// Load and return asset for given metadata. Returns nullptr on failure.
		static Ref<Asset> Import(const AssetMetadata& metadata);

	private:
		static Ref<Asset> ImportMesh    (const AssetMetadata& metadata);
		static Ref<Asset> ImportTexture (const AssetMetadata& metadata);
		static Ref<Asset> ImportMaterial(const AssetMetadata& metadata);
		static Ref<Asset> ImportPrefab(const AssetMetadata& metadata);
	};
}

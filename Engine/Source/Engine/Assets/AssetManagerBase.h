#pragma once
#include "Asset.h"

namespace Engine
{
	class AssetManagerBase
	{
	public:
		virtual ~AssetManagerBase() = default;

		virtual Ref<Asset> GetAsset(AssetHandle handle) = 0;

		virtual bool      IsAssetLoaded(AssetHandle handle) const = 0;
		virtual bool      IsAssetValid(AssetHandle handle)  const = 0;
		virtual AssetType GetAssetType(AssetHandle handle)  const = 0;
	};
}

#pragma once
#include "Engine/Debugging/Assert.h"

#include "AssetManagerBase.h"
#include "Engine/Project/Project.h"

namespace Engine
{
	class AssetManager
	{
	public:
		template<typename T>
		static Ref<T> GetAsset(AssetHandle handle)
		{
			Ref<Asset> asset = Project::GetActive()->GetAssetManager()->GetAsset(handle);
			if (!asset)
				return nullptr;

#ifdef KE_DEBUG
			ASSERT(asset->GetType() == T::GetStaticType(),
				"AssetManager::GetAsset<T> — type mismatch! Wrong asset type for handle.")
#endif
			return std::static_pointer_cast<T>(asset);
		}

		static bool IsAssetLoaded(AssetHandle handle)
		{
			return Project::GetActive()->GetAssetManager()->IsAssetLoaded(handle);
		}

		static bool IsAssetValid(AssetHandle handle)
		{
			return Project::GetActive()->GetAssetManager()->IsAssetValid(handle);
		}

		static AssetType GetAssetType(AssetHandle handle)
		{
			return Project::GetActive()->GetAssetManager()->GetAssetType(handle);
		}
	};
}

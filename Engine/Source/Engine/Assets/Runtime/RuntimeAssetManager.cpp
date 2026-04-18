#include "kepch.h"
#include "RuntimeAssetManager.h"

namespace Engine
{
	Ref<Asset> RuntimeAssetManager::GetAsset(AssetHandle handle)
	{
		auto it = m_Assets.find(handle);
		return it != m_Assets.end() ? it->second : nullptr;
	}

	bool RuntimeAssetManager::IsAssetLoaded(AssetHandle handle) const
	{
		return m_Assets.contains(handle);
	}

	bool RuntimeAssetManager::IsAssetValid(AssetHandle handle) const
	{
		return m_Assets.contains(handle);
	}

	AssetType RuntimeAssetManager::GetAssetType(AssetHandle handle) const
	{
		auto it = m_Assets.find(handle);
		return it != m_Assets.end() ? it->second->GetType() : AssetType::None;
	}
}

#include "kepch.h"
#include "AssetRegistry.h"

namespace Engine
{
	AssetMetadata& AssetRegistry::operator[](AssetHandle handle)
	{
		return m_Registry[handle];
	}

	const AssetMetadata* AssetRegistry::Get(AssetHandle handle) const
	{
		auto it = m_Registry.find(handle);
		return it != m_Registry.end() ? &it->second : nullptr;
	}

	AssetMetadata* AssetRegistry::Get(AssetHandle handle)
	{
		auto it = m_Registry.find(handle);
		return it != m_Registry.end() ? &it->second : nullptr;
	}

	bool AssetRegistry::Contains(AssetHandle handle) const
	{
		return m_Registry.contains(handle);
	}

	bool AssetRegistry::IsPathRegistered(const std::filesystem::path& relativePath) const
	{
		return m_PathToHandle.contains(relativePath);
	}

	AssetHandle AssetRegistry::GetHandleForPath(const std::filesystem::path& relativePath) const
	{
		auto it = m_PathToHandle.find(relativePath);
		return it != m_PathToHandle.end() ? it->second : AssetHandle(NullAssetHandle);
	}

	void AssetRegistry::Add(const AssetMetadata& metadata)
	{
		m_Registry[metadata.Handle]          = metadata;
		m_PathToHandle[metadata.FilePath]    = metadata.Handle;
	}

	void AssetRegistry::Remove(AssetHandle handle)
	{
		auto it = m_Registry.find(handle);
		if (it == m_Registry.end())
			return;

		m_PathToHandle.erase(it->second.FilePath);
		m_Registry.erase(it);
	}
}

#pragma once
#include "AssetMetadata.h"

namespace Engine
{
	class AssetRegistry
	{
	public:
		AssetMetadata&       operator[](AssetHandle handle);

		// Returns nullptr if not found
		const AssetMetadata* Get(AssetHandle handle) const;
		AssetMetadata*       Get(AssetHandle handle);

		bool                 Contains(AssetHandle handle) const;
		bool                 IsPathRegistered(const std::filesystem::path& relativePath) const;

		// O(1) — dual map kept in sync
		AssetHandle          GetHandleForPath(const std::filesystem::path& relativePath) const;

		void                 Add(const AssetMetadata& metadata);
		void                 Remove(AssetHandle handle);

		size_t               Count() const { return m_Registry.size(); }

		// Iteration — for Content Browser
		auto begin()       { return m_Registry.begin(); }
		auto end()         { return m_Registry.end(); }
		auto begin() const { return m_Registry.begin(); }
		auto end()   const { return m_Registry.end(); }

	private:
		std::unordered_map<AssetHandle, AssetMetadata>        m_Registry;
		std::unordered_map<std::filesystem::path, AssetHandle> m_PathToHandle;
	};
}

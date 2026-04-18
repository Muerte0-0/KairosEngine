#pragma once
#include "Asset.h"
#include <filesystem>

namespace Engine
{
	struct AssetMetadata
	{
		AssetHandle           Handle = AssetHandle(NullAssetHandle);
		AssetType             Type   = AssetType::None;

		// Relative to Project asset directory — never absolute, never with ./ or ../
		std::filesystem::path FilePath;

		bool IsValid() const { return Type != AssetType::None; }
	};
}

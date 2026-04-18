#pragma once
#include "Asset.h"
#include <filesystem>
#include <string>

namespace Engine
{
	struct AssetMetadata
	{
		AssetHandle           Handle          = AssetHandle(NullAssetHandle);
		AssetType             Type            = AssetType::None;

		// Relative to project asset directory — never absolute
		std::filesystem::path FilePath;

		// Populated from .kasset — used for reimport detection only
		std::string           SourceHash;
		uint32_t              ImporterVersion = 0;

		bool IsValid() const { return Type != AssetType::None; }
	};
}

#pragma once
#include "Asset.h"
#include <filesystem>
#include <string>

namespace Engine
{
	// Per-asset import toggles — stored in .kasset under ImportSettings
	struct TextureImportSettings
	{
		bool sRGB         = false;
		bool GenerateMips = false;
	};

	struct AssetMetadata
	{
		AssetHandle           Handle          = AssetHandle(NullAssetHandle);
		AssetType             Type            = AssetType::None;

		// Relative to project asset directory — never absolute
		std::filesystem::path FilePath;

		// Populated from .kasset — used for reimport detection only
		std::string           SourceHash;
		uint32_t              ImporterVersion = 0;

		// Type-specific import settings (only meaningful for Texture assets)
		TextureImportSettings TextureSettings;

		bool IsValid() const { return Type != AssetType::None; }
	};
}

#pragma once
#include "Engine/Assets/AssetMetadata.h"
#include <filesystem>

namespace Engine
{
	// .kasset sidecar: lives next to source file as <source>.kasset
	// e.g.  Barrel.fbx  ->  Barrel.fbx.kasset
	class AssetSerializer
	{
	public:
		AssetSerializer() = delete;

		static std::filesystem::path GetKassetPath(const std::filesystem::path& sourcePath);

		// Write .kasset next to sourcePath (absolute). Returns false on IO failure.
		static bool         Write(const std::filesystem::path& sourcePath, const AssetMetadata& metadata);

		// Read .kasset. Returns invalid metadata on parse failure.
		static AssetMetadata Read(const std::filesystem::path& kassetPath);

		// FNV-1a hash of source file bytes — for change detection.
		static std::string  ComputeSourceHash(const std::filesystem::path& sourcePath);

		static constexpr uint32_t CurrentImporterVersion = 1;
	};
}

#pragma once
#include "Engine/Core/UUID.h"

namespace Engine
{
	// Future: promote to struct for debug flags + overload control
	using AssetHandle = UUID;
	inline constexpr uint64_t NullAssetHandle = 0;

	enum class AssetType : uint16_t
	{
		None = 0,
		Mesh,
		Texture,
		Material,
		Shader,
		Scene,
	};

	class Asset
	{
	public:
		AssetHandle Handle = AssetHandle(NullAssetHandle);

		virtual AssetType GetType() const = 0;
		virtual ~Asset() = default;

		bool IsValid() const { return static_cast<uint64_t>(Handle) != NullAssetHandle; }
	};
}

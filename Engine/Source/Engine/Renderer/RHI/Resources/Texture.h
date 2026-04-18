#pragma once
#include "Engine/Core/Base.h"
#include "Engine/Utils/RendererUtils.h"
#include "Engine/Assets/Asset.h"
#include <filesystem>

namespace Engine {

	struct TextureSpecification
	{
		uint32_t		Width			{ 1 };
		uint32_t		Height			{ 1 };
		TextureFormat	Format			{ TextureFormat::RGBA8_UNorm };
		bool			GenerateMips	{ false };
	};

	class Texture : public Asset
	{
	public:
		static AssetType GetStaticType() { return AssetType::Texture; }
		AssetType        GetType() const override { return GetStaticType(); }

		virtual ~Texture() = default;

		virtual uint32_t		GetWidth()     const = 0;
		virtual uint32_t		GetHeight()    const = 0;
		virtual TextureFormat	GetFormat()    const = 0;
		virtual bool			IsLoaded()     const = 0;

		// Returns a backend-native handle (VkDescriptorSet on Vulkan).
		// Cast to ImTextureID at the call site. No ImGui dependency here.
		virtual uint64_t		GetTextureID() const = 0;

		// Load from file (stb_image)
		[[nodiscard]] static Ref<Texture> Create(const std::filesystem::path& path, const TextureSpecification& spec = {});

		// Create from raw pixel data
		[[nodiscard]] static Ref<Texture> Create(const void* data, uint32_t size, const TextureSpecification& spec);
	};

}

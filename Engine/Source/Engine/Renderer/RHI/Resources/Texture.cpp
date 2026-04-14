#include "kepch.h"
#include "Texture.h"
#include "Engine/Renderer/Renderer.h"
#include "APIs/Vulkan/VulkanTexture.h"

namespace Engine {

    Ref<Texture> Texture::Create(const std::filesystem::path& path, const TextureSpecification& spec)
    {
        switch (Renderer::GetAPI()->GetType())
        {
            case API::Vulkan: return CreateRef<VulkanTexture>(path, spec);
            default: ASSERT(false, "Texture::Create — unsupported API."); return nullptr;
        }
    }

    Ref<Texture> Texture::Create(const void* data, uint32_t size, const TextureSpecification& spec)
    {
        switch (Renderer::GetAPI()->GetType())
        {
            case API::Vulkan: return CreateRef<VulkanTexture>(data, size, spec);
            default: ASSERT(false, "Texture::Create — unsupported API."); return nullptr;
        }
    }
}

#include "kepch.h"
#include "MaterialFactory.h"

#include "Engine/Renderer/RHI/Resources/Texture.h"
#include "Engine/Renderer/Renderer.h"

// VulkanMaterial::Init() is called after textures are assigned.
// We reach it through the base Material::Bind() contract — Init() is
// invoked lazily on first Bind(), so no explicit call needed here.

namespace Engine
{
    std::vector<Ref<Material>> MaterialFactory::BuildAll(
        const std::vector<MaterialImportData>& importData)
    {
        std::unordered_map<std::string, Ref<Texture>> textureCache;
        std::vector<Ref<Material>> materials;
        materials.reserve(importData.size());

        for (const MaterialImportData& data : importData)
        {
            Ref<Material> mat = Material::Create();
            mat->SetName(data.Name);

            // Copy scalar params
            mat->Params.AlbedoColor   = data.AlbedoColor;
            mat->Params.Metallic      = data.Metallic;
            mat->Params.Roughness     = data.Roughness;

            // Load textures — empty path → slot stays null → fallback used at bind time
            mat->Albedo            = LoadTexture(data.AlbedoTexturePath,     textureCache);
            mat->Normal            = LoadTexture(data.NormalTexturePath,      textureCache);
            mat->MetallicRoughness = LoadTexture(data.MetallicRoughnessPath,  textureCache);
            mat->AO                = LoadTexture(data.AOTexturePath,          textureCache);
            mat->Emissive          = LoadTexture(data.EmissiveTexturePath,    textureCache);

            materials.push_back(std::move(mat));
        }

        return materials;
    }

    Ref<Texture> MaterialFactory::LoadTexture(
        const std::string& path,
        std::unordered_map<std::string, Ref<Texture>>& cache)
    {
        if (path.empty())
            return nullptr;

        auto it = cache.find(path);
        if (it != cache.end())
            return it->second;

        if (!std::filesystem::exists(path))
        {
            LOG(LogLevel::Warning, "MaterialFactory: texture not found: '{}'", path);
            return nullptr;
        }

        Ref<Texture> tex = Texture::Create(path);
        cache.emplace(path, tex);
        return tex;
    }

} // namespace Engine

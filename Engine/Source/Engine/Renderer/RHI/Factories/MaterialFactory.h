#pragma once
#include "Engine/Core/Base.h"
#include "Engine/Assets/AssimpModelLoader.h"
#include "Engine/Renderer/RHI/Resources/Material.h"

#include <string>
#include <vector>

namespace Engine
{
    // -----------------------------------------------------------------------
    // MaterialFactory
    //
    // Converts a vector of MaterialImportData (raw Assimp output) into
    // runtime Ref<Material> objects with GPU textures loaded.
    //
    // Texture cache prevents duplicate VulkanTexture allocation when the
    // same image path appears in multiple material slots.
    // -----------------------------------------------------------------------
    class MaterialFactory
    {
    public:
        MaterialFactory()  = delete;
        ~MaterialFactory() = delete;

        // Build one Material per entry in importData.
        // Relative paths in MaterialImportData are expected to be absolute
        // (resolved by AssimpModelLoader::ExtractTexturePath).
        [[nodiscard]] static std::vector<Ref<Material>> BuildAll(
            const std::vector<MaterialImportData>& importData);

    private:
        [[nodiscard]] static Ref<Material> Build(const MaterialImportData& data);

        // Per-call texture cache keyed on absolute path.
        // Passed by reference through Build() so each BuildAll() call shares it.
        [[nodiscard]] static Ref<Texture> LoadTexture(
            const std::string& path,
            std::unordered_map<std::string, Ref<Texture>>& cache);
    };

} // namespace Engine

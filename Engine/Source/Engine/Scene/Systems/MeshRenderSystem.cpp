#include "kepch.h"
#include "MeshRenderSystem.h"

#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Scene/Components.h"
#include "Engine/Assets/AssetManager.h"
#include "Engine/Materials/MaterialGraph.h"
#include "Engine/Materials/MaterialGraphCompiler.h"

namespace Engine::MeshRenderSystem
{
    // Compile (or return cached) material from a MaterialAsset.
    // Re-compiles whenever IsDirty is set.
    static Ref<Material> ResolveMaterialAsset(AssetHandle handle)
    {
        if (static_cast<uint64_t>(handle) == NullAssetHandle)
            return nullptr;

        Ref<MaterialAsset> asset = AssetManager::GetAsset<MaterialAsset>(handle);
        if (!asset)
            return nullptr;

        if (asset->IsDirty || !asset->CompiledMaterial)
        {
            asset->CompiledMaterial = MaterialGraphCompiler::Compile(asset->Graph);
            asset->IsDirty          = false;
        }

        return asset->CompiledMaterial;
    }

    void Render(entt::registry& registry, SceneRenderer& renderer)
    {
        auto view = registry.view<MeshComponent, TransformComponent>();

        for (auto entity : view)
        {
            const auto& [meshComp, transformComp] =
                view.get<MeshComponent, TransformComponent>(entity);

            if (!meshComp.HasMesh())
                continue;

            // If a .kmat override is assigned, compile it and use as sole material.
            Ref<Material> overrideMat = ResolveMaterialAsset(meshComp.MaterialAssetHandle);

            if (overrideMat)
            {
                // Override all submeshes with the compiled graph material
                std::vector<Ref<Material>> mats(
                    meshComp.MeshRef->GetSubMeshes().empty() ? 1
                                                             : meshComp.MeshRef->GetSubMeshes().size(),
                    overrideMat);
                renderer.SubmitMesh(meshComp.MeshRef, transformComp.GetTransform(), std::move(mats));
            }
            else
            {
                // Fallback: Assimp-imported materials (or empty → DrawMesh fallback white)
                renderer.SubmitMesh(meshComp.MeshRef, transformComp.GetTransform(), meshComp.Materials);
            }
        }
    }
}

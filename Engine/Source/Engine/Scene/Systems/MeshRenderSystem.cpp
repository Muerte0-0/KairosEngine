#include "kepch.h"
#include "MeshRenderSystem.h"

#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Scene/Components.h"
#include "Engine/Assets/AssetManager.h"
#include "Engine/Materials/MaterialGraph.h"
#include "Engine/Materials/MaterialGraphCompiler.h"

#include <glm/gtx/quaternion.hpp>

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
        // ---- Submit lights first ----------------------------------------
        auto lightView = registry.view<LightComponent, TransformComponent>();
        for (auto entity : lightView)
        {
            const auto& [lc, tc] = lightView.get<LightComponent, TransformComponent>(entity);

            LightSubmission ls{};
            ls.EntityID  = static_cast<uint32_t>(entt::to_integral(entity));
            ls.Color     = lc.Color;
            ls.Intensity = lc.Intensity;

            // Direction: -Y axis of the entity's rotation
            glm::mat4 rot = glm::toMat4(glm::quat(tc.Rotation));
            glm::vec3 downDir = glm::normalize(glm::vec3(rot * glm::vec4(0.f, -1.f, 0.f, 0.f)));

            switch (lc.Type)
            {
                case LightType::Directional:
                    ls.Type      = 0;
                    ls.Position  = glm::vec3(0.f);
                    ls.Direction = downDir;
                    ls.Range     = 0.f;
                    ls.InnerConeAngle = 0.f;
                    ls.OuterConeAngle = 0.f;
                    break;

                case LightType::Point:
                    ls.Type      = 1;
                    ls.Position  = tc.Translation;
                    ls.Direction = glm::vec3(0.f, -1.f, 0.f);
                    ls.Range     = lc.Point.Range;
                    ls.InnerConeAngle = 0.f;
                    ls.OuterConeAngle = 0.f;
                    break;

                case LightType::Spot:
                    ls.Type      = 2;
                    ls.Position  = tc.Translation;
                    ls.Direction = lc.Spot.Direction;
                    ls.Range     = lc.Spot.Range;
                    // Store as cosines — shader does dot(L,Dir) comparison directly
                    ls.InnerConeAngle = glm::cos(lc.Spot.InnerConeAngle);
                    ls.OuterConeAngle = glm::cos(lc.Spot.OuterConeAngle);
                    break;
            }

            renderer.SubmitLight(ls);
        }

        // ---- Submit meshes ----------------------------------------------
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

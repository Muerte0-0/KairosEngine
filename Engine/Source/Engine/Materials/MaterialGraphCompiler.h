#pragma once
#include "MaterialGraph.h"
#include "Engine/Core/Base.h"
#include "Engine/Renderer/RHI/Resources/Material.h"
#include "Engine/Renderer/RHI/Resources/Texture.h"

namespace Engine
{
    // -----------------------------------------------------------------------
    // MaterialGraphCompiler
    //
    // Walks a MaterialGraph and produces a Ref<Material> with GPU slots filled.
    //
    // PBROutputNode input → Material slot mapping:
    //   BaseColor  → Albedo texture (TextureSampleNode) OR Params.AlbedoColor (ConstantVec3)
    //   Normal     → Normal texture (TextureSampleNode only)
    //   Metallic   → Params.Metallic   (ConstantVec3.Value.x)
    //   Roughness  → Params.Roughness  (ConstantVec3.Value.x)
    //   AmbientOcc → AO texture       (TextureSampleNode only)
    //   Emissive   → Emissive texture  (TextureSampleNode only)
    //
    // Unconnected / unresolvable slots stay null — VulkanMaterial fallbacks apply.
    // -----------------------------------------------------------------------
    class MaterialGraphCompiler
    {
    public:
        MaterialGraphCompiler() = delete;

        // Returns a ready-to-Bind() Material, or nullptr if graph has no PBROutputNode.
        [[nodiscard]] static Ref<Material> Compile(const MaterialGraph& graph);

    private:
        static MaterialNode* ResolveSourceNode(const MaterialGraph& graph, uint32_t inputPinID);
        static Ref<Texture>  LoadTextureForNode(const TextureSampleNode& node);
    };

} // namespace Engine

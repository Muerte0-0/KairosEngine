#pragma once
#include "Engine/Core/Base.h"
#include "Engine/Utils/RendererUtils.h"
#include "Texture.h"

#include <glm/glm.hpp>
#include <string>

namespace Engine
{
    // -----------------------------------------------------------------------
    // MaterialParams — scalar PBR + layer properties.
    // Layout: std140 — each row is 16 bytes (vec4 or 4 floats).
    // Uploaded as UBO at set 1, binding 5.
    //
    // Row 0 : AlbedoColor        (vec4)
    // Row 1 : Metallic, Roughness, EmissiveScale, OrenNayarSigma
    // Row 2 : ClearCoat, ClearCoatRoughness, Wetness, _pad0
    // Row 3 : SpecularIntensity, SpecularTint, _pad1, _pad2
    // -----------------------------------------------------------------------
    struct alignas(16) MaterialParams
    {
        // Row 0
        glm::vec4 AlbedoColor        { 1.f, 1.f, 1.f, 1.f };

        // Row 1 — base layer
        float     Metallic           { 0.0f };
        float     Roughness          { 0.7f };
        float     EmissiveScale      { 0.0f };
        float     OrenNayarSigma     { 0.5f };   // 0=Lambertian, 1=max rough Oren-Nayar

        // Row 2 — augmentation layers
        float     ClearCoat          { 0.0f };   // 0=none, 1=full lacquer (IOR 1.5)
        float     ClearCoatRoughness { 0.0f };
        float     Wetness            { 0.0f };   // 0=dry, 1=wet
        float     _pad0              { 0.0f };

        // Row 3 — specular controls (Disney-style)
        // SpecularIntensity : multiplier on entire specular lobe. 0=no specular, 0.3=default.
        float     SpecularIntensity  { 0.3f };
        // SpecularTint : lerps achromatic F0 toward tinted baseColor. 0=white, 1=tinted.
        // Metals are always tinted; use for artistic control on dielectrics.
        float     SpecularTint       { 0.0f };
        float     _pad1              { 0.0f };
        float     _pad2              { 0.0f };
    };
    static_assert(sizeof(MaterialParams) % 16 == 0, "MaterialParams must be std140 aligned");

    // -----------------------------------------------------------------------
    // Material — RHI-agnostic base class.
    //
    // Texture binding convention (set 1):
    //   binding 0 — Albedo             (sRGB,   combined image sampler)
    //   binding 1 — Normal             (linear, combined image sampler)
    //   binding 2 — MetallicRoughness  (linear, combined image sampler)
    //   binding 3 — AO                 (linear, combined image sampler)
    //   binding 4 — Emissive           (sRGB,   combined image sampler)
    //   binding 5 — MaterialParams     (UBO)
    //
    // VulkanMaterial overrides Bind() → vkCmdBindDescriptorSets(set = 1).
    // -----------------------------------------------------------------------
    class Material
    {
    public:
        virtual ~Material() = default;

        // Bind for the given frame. Called per-SubMesh inside SceneRenderer::Flush().
        // commandBuffer / pipelineLayout are void* to stay Vulkan-free in this header.
        // Cast at the Vulkan impl side.
        virtual void Bind(
            void*    commandBuffer,
            void*    pipelineLayout,
            uint32_t frameIndex) = 0;

        // ---- Texture slots (null = fallback white/normal used by impl) ---
        Ref<Texture> Albedo;
        Ref<Texture> Normal;
        Ref<Texture> MetallicRoughness;
        Ref<Texture> AO;
        Ref<Texture> Emissive;

        // ---- Scalar params -----------------------------------------------
        MaterialParams Params;

        // ---- Identity ----------------------------------------------------
        const std::string& GetName() const   { return m_Name; }
        void SetName(std::string name)        { m_Name = std::move(name); }

        // ---- Factory -----------------------------------------------------
        // Dispatches to the active API backend (e.g. VulkanMaterial).
        [[nodiscard]] static Ref<Material> Create();

        // Returns a shared default white/rough material. Never null.
        [[nodiscard]] static Ref<Material> GetDefault();

        // Call during Renderer::Shutdown() to drop Vulkan resources before vkDestroyDevice.
        static void ResetDefault();

    protected:
        std::string m_Name;
    };

}

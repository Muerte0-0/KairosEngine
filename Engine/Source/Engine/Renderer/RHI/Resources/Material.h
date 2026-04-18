#pragma once
#include "Engine/Core/Base.h"
#include "Engine/Utils/RendererUtils.h"
#include "Texture.h"

#include <glm/glm.hpp>
#include <string>

namespace Engine
{
    // -----------------------------------------------------------------------
    // MaterialParams — scalar PBR properties.
    // std140: vec4 first, then scalars padded to 16-byte boundary.
    // Uploaded as UBO at set 1, binding 5.
    // -----------------------------------------------------------------------
    struct alignas(16) MaterialParams
    {
        glm::vec4 AlbedoColor   { 1.f, 1.f, 1.f, 1.f };
        float     Metallic      { 0.f };
        float     Roughness     { 1.f };
        float     EmissiveScale { 0.f };
        float     _pad          { 0.f };
    };
    static_assert(sizeof(MaterialParams) % 16 == 0, "MaterialParams must be std140 aligned");

    // -----------------------------------------------------------------------
    // Material — RHI-agnostic base class.
    //
    // Texture binding convention (set 1):
    //   binding 0 — Albedo            (sRGB,   combined image sampler)
    //   binding 1 — Normal            (linear, combined image sampler)
    //   binding 2 — MetallicRoughness (linear, combined image sampler)
    //   binding 3 — AO               (linear, combined image sampler)
    //   binding 4 — Emissive          (sRGB,   combined image sampler)
    //   binding 5 — MaterialParams    (UBO)
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

} // namespace Engine

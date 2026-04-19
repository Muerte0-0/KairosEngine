#pragma once
#include "Engine/Renderer/RHI/Resources/Material.h"

#include <vulkan/vulkan_raii.hpp>

namespace Engine
{
    // -----------------------------------------------------------------------
    // VulkanMaterial
    //
    // Owns one DescriptorPool and MAX_FRAMES_IN_FLIGHT descriptor sets for
    // set 1 (material textures + params UBO).
    //
    // Fallback 1x1 textures are shared across all instances (lazy init).
    // Init() must be called once after all texture slots are assigned
    // (or left null for fallbacks). SceneRenderer calls Init() via
    // MaterialFactory after building textures.
    //
    // Bind() is called per-SubMesh inside SceneRenderer::Flush().
    // -----------------------------------------------------------------------
    class VulkanMaterial final : public Material
    {
    public:
        VulkanMaterial();
        ~VulkanMaterial() override;

        // Build GPU resources (pool, sets, UBOs, write descriptors).
        // Call once after texture slots are populated.
        // Safe to call again after texture changes — rebuilds writes only.
        void Init();

        // vk::CommandBuffer* / vk::PipelineLayout* passed as void* to keep
        // the base class Vulkan-free.
        void Bind(void* commandBuffer, void* pipelineLayout, uint32_t frameIndex) override;

        // Release all static fallback textures — call before vkDestroyDevice.
        static void ResetStaticResources();

    private:
        // Per-frame UBO for MaterialParams
        std::vector<vk::raii::Buffer>       m_ParamBuffers;
        std::vector<vk::raii::DeviceMemory> m_ParamMemory;
        std::vector<void*>                  m_ParamMapped;

        vk::raii::DescriptorPool                 m_Pool = nullptr;
        std::vector<vk::raii::DescriptorSet>     m_Sets;

        bool m_Initialised { false };

        void CreatePool();
        void CreateParamBuffers();
        void AllocateSets();
        void WriteDescriptors();

        // Returns the fallback for a given slot (lazily created 1x1 white/normal textures).
        static Ref<Texture> GetFallbackAlbedo();
        static Ref<Texture> GetFallbackNormal();
        static Ref<Texture> GetFallbackBlack();          // emissive
        static Ref<Texture> GetFallbackMetallicRough();  // G=255(rough=1) B=0(metal=0)
    };

} // namespace Engine

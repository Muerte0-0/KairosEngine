#include "kepch.h"
#include "VulkanMaterial.h"

#include "VulkanRenderAPI.h"
#include "VulkanUtils.h"
#include "VulkanTexture.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/RHI/Resources/Texture.h"

namespace Engine
{
    namespace
    {
        VulkanRenderAPI* GetAPI()
        {
            auto* api = dynamic_cast<VulkanRenderAPI*>(Renderer::GetAPI());
            ASSERT(api, "VulkanMaterial: active RenderAPI is not VulkanRenderAPI.");
            return api;
        }

        vk::raii::Device& GetDevice() { return GetAPI()->GetVulkanDevice()->GetDevice(); }
    }

    // -----------------------------------------------------------------------
    // Fallback textures — file-scope so ResetStaticResources() can null them
    // -----------------------------------------------------------------------
    static Ref<Texture> s_FallbackAlbedo;
    static Ref<Texture> s_FallbackNormal;
    static Ref<Texture> s_FallbackBlack;

    Ref<Texture> VulkanMaterial::GetFallbackAlbedo()
    {
        if (!s_FallbackAlbedo)
        {
            constexpr uint8_t px[4] = { 255, 255, 255, 255 };
            TextureSpecification spec; spec.Width = 1; spec.Height = 1;
            s_FallbackAlbedo = CreateRef<VulkanTexture>(px, sizeof(px), spec);
        }
        return s_FallbackAlbedo;
    }

    Ref<Texture> VulkanMaterial::GetFallbackNormal()
    {
        if (!s_FallbackNormal)
        {
            constexpr uint8_t px[4] = { 128, 128, 255, 255 };
            TextureSpecification spec; spec.Width = 1; spec.Height = 1;
            s_FallbackNormal = CreateRef<VulkanTexture>(px, sizeof(px), spec);
        }
        return s_FallbackNormal;
    }

    Ref<Texture> VulkanMaterial::GetFallbackBlack()
    {
        if (!s_FallbackBlack)
        {
            constexpr uint8_t px[4] = { 0, 0, 0, 255 };
            TextureSpecification spec; spec.Width = 1; spec.Height = 1;
            s_FallbackBlack = CreateRef<VulkanTexture>(px, sizeof(px), spec);
        }
        return s_FallbackBlack;
    }

    // -----------------------------------------------------------------------
    // Static resource cleanup — call before vkDestroyDevice
    // -----------------------------------------------------------------------
    void VulkanMaterial::ResetStaticResources()
    {
        s_FallbackAlbedo.reset();
        s_FallbackNormal.reset();
        s_FallbackBlack.reset();
    }

    // -----------------------------------------------------------------------
    // Construction / destruction
    // -----------------------------------------------------------------------
    VulkanMaterial::VulkanMaterial()  = default;
    VulkanMaterial::~VulkanMaterial() = default;

    // -----------------------------------------------------------------------
    // Init — call once after texture slots are set
    // -----------------------------------------------------------------------
    void VulkanMaterial::Init()
    {
        if (!m_Initialised)
        {
            CreatePool();
            CreateParamBuffers();
            AllocateSets();
        }
        WriteDescriptors();   // always re-write so texture swaps take effect
        m_Initialised = true;
    }

    void VulkanMaterial::CreatePool()
    {
        // 5 samplers * MAX_FRAMES + 1 UBO * MAX_FRAMES
        std::array poolSizes = {
            vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler,
                                   5 * MAX_FRAMES_IN_FLIGHT),
            vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer,
                                   MAX_FRAMES_IN_FLIGHT),
        };
        vk::DescriptorPoolCreateInfo info(
            vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
            MAX_FRAMES_IN_FLIGHT, poolSizes);
        m_Pool = vk::raii::DescriptorPool(GetDevice(), info);
    }

    void VulkanMaterial::CreateParamBuffers()
    {
        m_ParamBuffers.clear();
        m_ParamMemory.clear();
        m_ParamMapped.clear();

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            vk::raii::Buffer       buf({});
            vk::raii::DeviceMemory mem({});
            VulkanUtils::CreateBuffer(
                GetDevice(),
                GetAPI()->GetVulkanDevice()->GetPhysicalDevice(),
                sizeof(MaterialParams),
                vk::BufferUsageFlagBits::eUniformBuffer,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                buf, mem);
            m_ParamMapped.push_back(mem.mapMemory(0, sizeof(MaterialParams)));
            m_ParamBuffers.emplace_back(std::move(buf));
            m_ParamMemory.emplace_back(std::move(mem));
        }
    }

    void VulkanMaterial::AllocateSets()
    {
        auto& dsl = GetAPI()->GetMaterialDescriptorSetLayout();
        std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *dsl);
        vk::DescriptorSetAllocateInfo info(*m_Pool, layouts);
        m_Sets = GetDevice().allocateDescriptorSets(info);
    }

    void VulkanMaterial::WriteDescriptors()
    {
        // Resolve textures — fall back if slot is null
        auto ResolveImg = [](const Ref<Texture>& t, const Ref<Texture>& fallback) -> const VulkanTexture*
        {
            const Texture* src = (t && t->IsLoaded()) ? t.get() : fallback.get();
            return static_cast<const VulkanTexture*>(src);
        };

        const VulkanTexture* albedo  = ResolveImg(Albedo,           GetFallbackAlbedo());
        const VulkanTexture* normal  = ResolveImg(Normal,           GetFallbackNormal());
        const VulkanTexture* mr      = ResolveImg(MetallicRoughness, GetFallbackBlack());
        const VulkanTexture* ao      = ResolveImg(AO,               GetFallbackAlbedo()); // white AO
        const VulkanTexture* emissive= ResolveImg(Emissive,         GetFallbackBlack());

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            // Upload params
            memcpy(m_ParamMapped[i], &Params, sizeof(MaterialParams));

            // Image infos
            auto MakeImgInfo = [](const VulkanTexture* t) {
                return vk::DescriptorImageInfo(
                    t->GetSampler(), t->GetImageView(),
                    vk::ImageLayout::eShaderReadOnlyOptimal);
            };
            std::array imgInfos = {
                MakeImgInfo(albedo), MakeImgInfo(normal),
                MakeImgInfo(mr),     MakeImgInfo(ao),
                MakeImgInfo(emissive),
            };

            vk::DescriptorBufferInfo bufInfo(*m_ParamBuffers[i], 0, sizeof(MaterialParams));

            std::array<vk::WriteDescriptorSet, 6> writes{};
            for (uint32_t b = 0; b < 5; ++b)
            {
                writes[b].dstSet          = *m_Sets[i];
                writes[b].dstBinding      = b;
                writes[b].descriptorCount = 1;
                writes[b].descriptorType  = vk::DescriptorType::eCombinedImageSampler;
                writes[b].pImageInfo      = &imgInfos[b];
            }
            writes[5].dstSet          = *m_Sets[i];
            writes[5].dstBinding      = 5;
            writes[5].descriptorCount = 1;
            writes[5].descriptorType  = vk::DescriptorType::eUniformBuffer;
            writes[5].pBufferInfo     = &bufInfo;

            GetDevice().updateDescriptorSets(writes, {});
        }
    }

    // -----------------------------------------------------------------------
    // Bind — called per-SubMesh in SceneRenderer::Flush()
    // -----------------------------------------------------------------------
    void VulkanMaterial::Bind(void* commandBuffer, void* pipelineLayout, uint32_t frameIndex)
    {
        if (!m_Initialised)
            Init();

        // Update params for this frame
        memcpy(m_ParamMapped[frameIndex], &Params, sizeof(MaterialParams));

        auto cmd    = static_cast<vk::CommandBuffer*>(commandBuffer);
        auto layout = static_cast<vk::PipelineLayout*>(pipelineLayout);

        cmd->bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            *layout,
            1,                       // set = 1
            *m_Sets[frameIndex],
            nullptr);
    }

} // namespace Engine

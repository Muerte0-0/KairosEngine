#include "kepch.h"

#include "VulkanTexture.h"

#include "Engine/Renderer/Renderer.h"

#include "VulkanRenderAPI.h"
#include "VulkanUtils.h"

#include "Components/VulkanDevice.h"
#include "Components/VulkanCommand.h"

#include "stb_image.h"

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>

namespace Engine
{
    VulkanTexture::VulkanTexture(const std::filesystem::path& path, const TextureSpecification& spec) : m_Spec(spec)
    {
        int w, h, channels;
        stbi_set_flip_vertically_on_load(0);
        stbi_uc* pixels = stbi_load(path.string().c_str(), &w, &h, &channels, STBI_rgb_alpha);

        ASSERT(pixels, "VulkanTexture: stbi_load failed for '{}'.", path.string());

        m_Spec.Width  = static_cast<uint32_t>(w);
        m_Spec.Height = static_cast<uint32_t>(h);
        m_Spec.Format = TextureFormat::RGBA8_UNorm;

        uint32_t dataSize = m_Spec.Width * m_Spec.Height * 4;
        UploadToGPU(pixels, dataSize);
        stbi_image_free(pixels);

        LOG(LogLevel::Info, "VulkanTexture: loaded '{}' ({}x{})", path.filename().string(), w, h);
    }

    VulkanTexture::VulkanTexture(const void* data, uint32_t size, const TextureSpecification& spec) : m_Spec(spec)
    {
        UploadToGPU(data, size);
        // m_Loaded set inside UploadToGPU
        // LOG(LogLevel::Info, "VulkanTexture: loaded from raw data ({}x{})", m_Spec.Width, m_Spec.Height);
    }

    VulkanTexture::~VulkanTexture()
    {
        ReleaseTextureID();
        // raii members destroy themselves
    }

    void VulkanTexture::UploadToGPU(const void* pixelData, uint32_t dataSize)
    {
        auto* api = dynamic_cast<VulkanRenderAPI*>(Renderer::GetAPI());
        auto& device   = api->GetVulkanDevice()->GetDevice();
        auto& physDev  = api->GetVulkanDevice()->GetPhysicalDevice();

        // Staging buffer
        vk::raii::Buffer       stagingBuf({});
        vk::raii::DeviceMemory stagingMem({});
        VulkanUtils::CreateBuffer(device, physDev, dataSize,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
            stagingBuf, stagingMem);

        void* mapped = stagingMem.mapMemory(0, dataSize);
        memcpy(mapped, pixelData, dataSize);
        stagingMem.unmapMemory();

        // Device-local image
        VulkanUtils::CreateImage(device, physDev,
            m_Spec.Width, m_Spec.Height, 1,
            vk::SampleCountFlagBits::e1,
            VulkanUtils::ToVulkanFormat(m_Spec.Format),
            vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
            vk::MemoryPropertyFlagBits::eDeviceLocal,
            m_Image, m_ImageMemory);

        // Transition -> TransferDst, copy, transition -> ShaderReadOnly
        auto cmdBuf = VulkanUtils::BeginSingleTimeCommands(device, api->GetVulkanCommand()->GetCommandPool());

        VulkanUtils::TransitionImageLayout(*cmdBuf, *m_Image,
            vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
            {}, vk::AccessFlagBits2::eTransferWrite,
            vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eTransfer,
            vk::ImageAspectFlagBits::eColor);

        // Copy buffer -> image
        vk::BufferImageCopy region;
        region.bufferOffset      = 0;
        region.bufferRowLength   = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource  = { vk::ImageAspectFlagBits::eColor, 0, 0, 1 };
        region.imageOffset       = vk::Offset3D(0, 0, 0);
        region.imageExtent       = vk::Extent3D(m_Spec.Width, m_Spec.Height, 1);
        cmdBuf->copyBufferToImage(*stagingBuf, *m_Image, vk::ImageLayout::eTransferDstOptimal, region);

        VulkanUtils::TransitionImageLayout(*cmdBuf, *m_Image,
            vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
            vk::AccessFlagBits2::eTransferWrite, vk::AccessFlagBits2::eShaderRead,
            vk::PipelineStageFlagBits2::eTransfer, vk::PipelineStageFlagBits2::eFragmentShader,
            vk::ImageAspectFlagBits::eColor);

        VulkanUtils::EndSingleTimeCommands(*cmdBuf, api->GetVulkanDevice()->GetGraphicsQueue());

        // ImageView + Sampler
        m_ImageView = VulkanUtils::CreateImageView(device, m_Image,
            VulkanUtils::ToVulkanFormat(m_Spec.Format),
            vk::ImageAspectFlagBits::eColor, 1);

        CreateSampler();
        m_Loaded = true;
    }

    void VulkanTexture::CreateSampler()
    {
        auto* api = dynamic_cast<VulkanRenderAPI*>(Renderer::GetAPI());
        vk::SamplerCreateInfo info;
        info.magFilter        = vk::Filter::eLinear;
        info.minFilter        = vk::Filter::eLinear;
        info.addressModeU     = vk::SamplerAddressMode::eRepeat;
        info.addressModeV     = vk::SamplerAddressMode::eRepeat;
        info.addressModeW     = vk::SamplerAddressMode::eRepeat;
        info.anisotropyEnable = vk::True;
        info.maxAnisotropy    = api->GetVulkanDevice()->GetPhysicalDevice()
                                    .getProperties().limits.maxSamplerAnisotropy;
        info.borderColor      = vk::BorderColor::eIntOpaqueBlack;
        info.mipmapMode       = vk::SamplerMipmapMode::eLinear;

        m_Sampler = vk::raii::Sampler(api->GetVulkanDevice()->GetDevice(), info);
    }

    uint64_t VulkanTexture::GetTextureID() const
    {        
        if (!m_TextureID && ImGui::GetCurrentContext())
        {
            VkDescriptorSet ds = ImGui_ImplVulkan_AddTexture(
                *m_Sampler, *m_ImageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            m_TextureID = reinterpret_cast<uint64_t>(ds);
        }
        return m_TextureID;
    }

    void VulkanTexture::ReleaseTextureID()
    {
        if (m_TextureID && ImGui::GetCurrentContext())
        {
            ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(m_TextureID));
            m_TextureID = 0;
        }
    }

}

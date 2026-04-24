#include "kepch.h"
#include "VulkanFramebuffer.h"

#include "Components/VulkanDevice.h"
#include "VulkanUtils.h"

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

namespace Engine
{
	// -----------------------------------------------------------------------
	// Helpers
	// -----------------------------------------------------------------------

	namespace
	{
		vk::SampleCountFlagBits PickMSAASamples(VulkanDevice& device, SampleCountBits requested)
		{
			if (requested == SampleCountBits::s1)
				return vk::SampleCountFlagBits::e1;

			vk::PhysicalDeviceProperties props = device.GetPhysicalDevice().getProperties();
			vk::SampleCountFlags supported =
				props.limits.framebufferColorSampleCounts &
				props.limits.framebufferDepthSampleCounts;

			vk::SampleCountFlagBits desired = VulkanUtils::ToVulkanSampleCount(requested);

			// Walk down from the desired count until we find one the device supports.
			constexpr vk::SampleCountFlagBits kOrder[] = {
				vk::SampleCountFlagBits::e64, vk::SampleCountFlagBits::e32,
				vk::SampleCountFlagBits::e16, vk::SampleCountFlagBits::e8,
				vk::SampleCountFlagBits::e4,  vk::SampleCountFlagBits::e2,
				vk::SampleCountFlagBits::e1,
			};

			bool found = false;
			for (auto s : kOrder)
			{
				if (s == desired) found = true;
				if (found && (supported & s))
					return s;
			}
			return vk::SampleCountFlagBits::e1;
		}

		vk::Format FindDepthFormat(VulkanDevice& device)
		{
			return VulkanUtils::FindSupportedFormat(
				device.GetPhysicalDevice(),
				{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
				vk::ImageTiling::eOptimal,
				vk::FormatFeatureFlagBits::eDepthStencilAttachment);
		}
	}

	// -----------------------------------------------------------------------
	// Construction / destruction
	// -----------------------------------------------------------------------

	VulkanFramebuffer::VulkanFramebuffer(VulkanDevice& device, const FramebufferSpecification& spec)
		: m_Device(device), m_FramebufferSpec(spec)
	{
		m_MSAASamples = PickMSAASamples(m_Device, m_FramebufferSpec.SampleCount);

		if (HasColorAttachment())
			CreateColorAttachment();
		if (HasColorAttachment() && HasMSAA())
			CreateMSAAColorAttachment();
		if (HasDepthAttachment())
			CreateDepthAttachment();
		if (HasColorAttachment())
			CreateColorSampler();
		if (HasDepthAttachment())
			CreateDepthSampler();
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
		ReleaseImGuiTexture();
	}

	// -----------------------------------------------------------------------
	// Public API
	// -----------------------------------------------------------------------

	void VulkanFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		width  = (std::max)(width,  1u);
		height = (std::max)(height, 1u);

		if (m_FramebufferSpec.Width == width && m_FramebufferSpec.Height == height)
			return;

		m_Device.WaitIdle();
		ReleaseImGuiTexture();
		ReleaseResources();

		m_FramebufferSpec.Width  = width;
		m_FramebufferSpec.Height = height;

		if (HasColorAttachment())
			CreateColorAttachment();
		if (HasColorAttachment() && HasMSAA())
			CreateMSAAColorAttachment();
		if (HasDepthAttachment())
			CreateDepthAttachment();
	}

	void* VulkanFramebuffer::GetImGuiTextureID()
	{
		if (!HasColorAttachment())
			return nullptr;

		if (m_ImGuiTextureID == nullptr && ImGui::GetCurrentContext() != nullptr)
		{
			m_ImGuiTextureID = ImGui_ImplVulkan_AddTexture(
				*m_ColorSampler,
				*m_ColorImageView,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		return m_ImGuiTextureID;
	}

	vk::RenderingInfo VulkanFramebuffer::BuildRenderingInfo(const glm::vec4& clearColor) const
	{
		static vk::RenderingAttachmentInfo colorAttachment{};
		if (HasColorAttachment())
		{
			colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
			colorAttachment.loadOp      = vk::AttachmentLoadOp::eClear;
			colorAttachment.storeOp     = vk::AttachmentStoreOp::eStore;
			colorAttachment.clearValue  = vk::ClearColorValue(std::array<float, 4>{
				clearColor.r, clearColor.g, clearColor.b, clearColor.a });

			if (HasMSAA())
			{
				colorAttachment.imageView          = *m_MSAAColorImageView;
				colorAttachment.resolveMode        = vk::ResolveModeFlagBits::eAverage;
				colorAttachment.resolveImageView   = *m_ColorImageView;
				colorAttachment.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
			}
			else
			{
				colorAttachment.imageView          = *m_ColorImageView;
				colorAttachment.resolveMode        = vk::ResolveModeFlagBits::eNone;
				colorAttachment.resolveImageView   = nullptr;
				colorAttachment.resolveImageLayout = vk::ImageLayout::eUndefined;
			}
		}

		static vk::RenderingAttachmentInfo depthAttachment{};
		if (HasDepth())
		{
			depthAttachment.imageView   = *m_DepthImageView;
			depthAttachment.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			depthAttachment.loadOp      = vk::AttachmentLoadOp::eClear;
			depthAttachment.storeOp     = vk::AttachmentStoreOp::eStore;
			depthAttachment.clearValue  = vk::ClearDepthStencilValue(1.0f, 0);
		}

		vk::RenderingInfo info;
		info.renderArea           = vk::Rect2D(vk::Offset2D(0, 0), GetExtent());
		info.layerCount           = 1;
		info.colorAttachmentCount = HasColorAttachment() ? 1u : 0u;
		info.pColorAttachments    = HasColorAttachment() ? &colorAttachment : nullptr;
		info.pDepthAttachment     = HasDepth() ? &depthAttachment : nullptr;

		return info;
	}

	// -----------------------------------------------------------------------
	// Private helpers
	// -----------------------------------------------------------------------

	void VulkanFramebuffer::CreateColorAttachment()
	{
		VulkanUtils::CreateImage(
			m_Device.GetDevice(),
			m_Device.GetPhysicalDevice(),
			m_FramebufferSpec.Width,
			m_FramebufferSpec.Height,
			1,
			vk::SampleCountFlagBits::e1,   // resolve target is always 1x
			VulkanUtils::ToVulkanFormat(m_FramebufferSpec.ColorFormat),
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			m_ColorImage,
			m_ColorImageMemory);

		m_ColorImageView = VulkanUtils::CreateImageView(
			m_Device.GetDevice(),
			m_ColorImage,
			VulkanUtils::ToVulkanFormat(m_FramebufferSpec.ColorFormat),
			vk::ImageAspectFlagBits::eColor,
			1);

		m_CurrentColorLayout = vk::ImageLayout::eUndefined;
	}

	void VulkanFramebuffer::CreateMSAAColorAttachment()
	{
		VulkanUtils::CreateImage(
			m_Device.GetDevice(),
			m_Device.GetPhysicalDevice(),
			m_FramebufferSpec.Width,
			m_FramebufferSpec.Height,
			1,
			m_MSAASamples,
			VulkanUtils::ToVulkanFormat(m_FramebufferSpec.ColorFormat),
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			m_MSAAColorImage,
			m_MSAAColorImageMemory);

		m_MSAAColorImageView = VulkanUtils::CreateImageView(
			m_Device.GetDevice(),
			m_MSAAColorImage,
			VulkanUtils::ToVulkanFormat(m_FramebufferSpec.ColorFormat),
			vk::ImageAspectFlagBits::eColor,
			1);
	}

	void VulkanFramebuffer::CreateDepthAttachment()
	{
		// Use the spec's depth format if it maps to a real Vulkan format;
		// otherwise query the device for a suitable one.
		vk::Format depthFmt = VulkanUtils::ToVulkanFormat(m_FramebufferSpec.DepthFormat);
		if (depthFmt == vk::Format::eUndefined)
			depthFmt = FindDepthFormat(m_Device);

		VulkanUtils::CreateImage(
			m_Device.GetDevice(),
			m_Device.GetPhysicalDevice(),
			m_FramebufferSpec.Width,
			m_FramebufferSpec.Height,
			1,
			m_MSAASamples,   // depth must match the color sample count
			depthFmt,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			m_DepthImage,
			m_DepthImageMemory);

		m_DepthImageView = VulkanUtils::CreateImageView(
			m_Device.GetDevice(),
			m_DepthImage,
			depthFmt,
			vk::ImageAspectFlagBits::eDepth,
			1);

		m_CurrentDepthLayout = vk::ImageLayout::eUndefined;
	}

	void VulkanFramebuffer::CreateColorSampler()
	{
		vk::SamplerCreateInfo samplerInfo;
		samplerInfo.magFilter               = vk::Filter::eLinear;
		samplerInfo.minFilter               = vk::Filter::eLinear;
		samplerInfo.addressModeU            = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeV            = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeW            = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.anisotropyEnable        = vk::False;
		samplerInfo.maxAnisotropy           = 1.0f;
		samplerInfo.borderColor             = vk::BorderColor::eFloatOpaqueWhite;
		samplerInfo.unnormalizedCoordinates = vk::False;
		samplerInfo.compareEnable           = vk::False;
		samplerInfo.mipmapMode              = vk::SamplerMipmapMode::eLinear;

		m_ColorSampler = vk::raii::Sampler(m_Device.GetDevice(), samplerInfo);
	}

	void VulkanFramebuffer::CreateDepthSampler()
	{
		vk::SamplerCreateInfo samplerInfo;
		samplerInfo.magFilter               = vk::Filter::eLinear;
		samplerInfo.minFilter               = vk::Filter::eLinear;
		samplerInfo.addressModeU            = vk::SamplerAddressMode::eClampToBorder;
		samplerInfo.addressModeV            = vk::SamplerAddressMode::eClampToBorder;
		samplerInfo.addressModeW            = vk::SamplerAddressMode::eClampToBorder;
		samplerInfo.anisotropyEnable        = vk::False;
		samplerInfo.maxAnisotropy           = 1.0f;
		samplerInfo.borderColor             = vk::BorderColor::eFloatOpaqueWhite;
		samplerInfo.unnormalizedCoordinates = vk::False;
		samplerInfo.compareEnable           = vk::False;
		samplerInfo.mipmapMode              = vk::SamplerMipmapMode::eLinear;

		m_DepthSampler = vk::raii::Sampler(m_Device.GetDevice(), samplerInfo);
	}

	void VulkanFramebuffer::ReleaseResources()
	{
		// Color resolve
		m_ColorImageView   = nullptr;
		m_ColorImage       = nullptr;
		m_ColorImageMemory = nullptr;
		m_CurrentColorLayout = vk::ImageLayout::eUndefined;

		// MSAA
		m_MSAAColorImageView   = nullptr;
		m_MSAAColorImage       = nullptr;
		m_MSAAColorImageMemory = nullptr;

		// Depth
		m_DepthImageView   = nullptr;
		m_DepthImage       = nullptr;
		m_DepthImageMemory = nullptr;
		m_CurrentDepthLayout = vk::ImageLayout::eUndefined;
	}

	void VulkanFramebuffer::ReleaseImGuiTexture()
	{
		if (m_ImGuiTextureID != nullptr && ImGui::GetCurrentContext() != nullptr)
			ImGui_ImplVulkan_RemoveTexture(static_cast<VkDescriptorSet>(m_ImGuiTextureID));

		m_ImGuiTextureID = nullptr;
	}
}

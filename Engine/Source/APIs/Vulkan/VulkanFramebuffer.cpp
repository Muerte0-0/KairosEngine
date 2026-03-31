#include "kepch.h"
#include "VulkanFramebuffer.h"

#include "Components/VulkanDevice.h"
#include "VulkanUtils.h"

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

namespace Engine
{
	VulkanFramebuffer::VulkanFramebuffer(VulkanDevice& device, FramebufferSpecification specification, vk::Format colorFormat)
		: Framebuffer(specification), m_Device(device), m_ColorFormat(colorFormat)
	{
		CreateColorAttachment();
		CreateSampler();
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
		ReleaseImGuiTexture();
	}

	void VulkanFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		width = (std::max)(width, 1u);
		height = (std::max)(height, 1u);

		if (m_Specification.Width == width && m_Specification.Height == height)
		{
			return;
		}

		ReleaseImGuiTexture();

		m_Specification.Width = width;
		m_Specification.Height = height;

		m_ColorImageView = nullptr;
		m_ColorImage = nullptr;
		m_ColorImageMemory = nullptr;
		m_CurrentLayout = vk::ImageLayout::eUndefined;

		CreateColorAttachment();
	}

	void* VulkanFramebuffer::GetImGuiTextureID()
	{
		if (m_ImGuiTextureID == nullptr && ImGui::GetCurrentContext() != nullptr)
		{
			m_ImGuiTextureID = ImGui_ImplVulkan_AddTexture(*m_ColorSampler, *m_ColorImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}

		return m_ImGuiTextureID;
	}

	void VulkanFramebuffer::CreateColorAttachment()
	{
		VulkanUtils::CreateImage(
			m_Device.GetDevice(),
			m_Device.GetPhysicalDevice(),
			m_Specification.Width,
			m_Specification.Height,
			1,
			vk::SampleCountFlagBits::e1,
			m_ColorFormat,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			m_ColorImage,
			m_ColorImageMemory);

		m_ColorImageView = VulkanUtils::CreateImageView(m_Device.GetDevice(), m_ColorImage, m_ColorFormat, vk::ImageAspectFlagBits::eColor, 1);
	}

	void VulkanFramebuffer::CreateSampler()
	{
		vk::SamplerCreateInfo samplerInfo;
		samplerInfo.magFilter = vk::Filter::eLinear;
		samplerInfo.minFilter = vk::Filter::eLinear;
		samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.anisotropyEnable = vk::False;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
		samplerInfo.unnormalizedCoordinates = vk::False;
		samplerInfo.compareEnable = vk::False;
		samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;

		m_ColorSampler = vk::raii::Sampler(m_Device.GetDevice(), samplerInfo);
	}

	void VulkanFramebuffer::ReleaseImGuiTexture()
	{
		if (m_ImGuiTextureID != nullptr && ImGui::GetCurrentContext() != nullptr)
		{
			ImGui_ImplVulkan_RemoveTexture(static_cast<VkDescriptorSet>(m_ImGuiTextureID));
		}

		m_ImGuiTextureID = nullptr;
	}
}

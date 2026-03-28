#pragma once

#include "Engine/Renderer/RHI/Framebuffer.h"

#include <vulkan/vulkan_raii.hpp>

namespace Engine
{
	class VulkanDevice;

	class VulkanFramebuffer : public Framebuffer
	{
	public:
		VulkanFramebuffer(VulkanDevice& device, FramebufferSpecification specification, vk::Format colorFormat);
		~VulkanFramebuffer() override;

		void Resize(uint32_t width, uint32_t height) override;
		void* GetImGuiTextureID() override;

		vk::Image GetImage() const { return *m_ColorImage; }
		vk::ImageView GetImageView() const { return *m_ColorImageView; }
		vk::Sampler GetSampler() const { return *m_ColorSampler; }
		vk::Format GetColorFormat() const { return m_ColorFormat; }
		vk::Extent2D GetExtent() const { return vk::Extent2D(m_Specification.Width, m_Specification.Height); }
		vk::ImageLayout GetCurrentLayout() const { return m_CurrentLayout; }
		void SetCurrentLayout(vk::ImageLayout layout) { m_CurrentLayout = layout; }

	private:
		void CreateColorAttachment();
		void CreateSampler();
		void ReleaseImGuiTexture();

		VulkanDevice& m_Device;
		vk::Format m_ColorFormat = vk::Format::eUndefined;
		vk::ImageLayout m_CurrentLayout = vk::ImageLayout::eUndefined;

		vk::raii::Image m_ColorImage = nullptr;
		vk::raii::DeviceMemory m_ColorImageMemory = nullptr;
		vk::raii::ImageView m_ColorImageView = nullptr;
		vk::raii::Sampler m_ColorSampler = nullptr;
		void* m_ImGuiTextureID = nullptr;
	};
}

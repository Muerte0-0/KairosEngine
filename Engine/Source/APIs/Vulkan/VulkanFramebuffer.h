#pragma once

#include "Engine/Renderer/RHI/Framebuffer.h"

#include <vulkan/vulkan_raii.hpp>

namespace Engine
{
	class VulkanDevice;

	class VulkanFramebuffer : public Framebuffer
	{
	public:
		VulkanFramebuffer(const FramebufferSpecification& spec);
		~VulkanFramebuffer() override;

		void Resize(uint32_t width, uint32_t height) override;
		void* GetImGuiTextureID() override;
		
		uint32_t GetWidth() const override { return m_FramebufferSpec.Width; }
		uint32_t GetHeight() const override { return m_FramebufferSpec.Height; }

		vk::Image GetImage() const { return *m_ColorImage; }
		vk::ImageView GetImageView() const { return *m_ColorImageView; }
		vk::Sampler GetSampler() const { return *m_ColorSampler; }
		vk::Extent2D GetExtent() const { return vk::Extent2D(m_FramebufferSpec.Width, m_FramebufferSpec.Height); }
		vk::ImageLayout GetCurrentLayout() const { return m_CurrentLayout; }
		void SetCurrentLayout(vk::ImageLayout layout) { m_CurrentLayout = layout; }

	private:
		void CreateColorAttachment();
		void CreateSampler();
		void ReleaseImGuiTexture();

		vk::ImageLayout m_CurrentLayout = vk::ImageLayout::eUndefined;

		FramebufferSpecification m_FramebufferSpec;
		
		vk::raii::Image m_ColorImage = nullptr;
		vk::raii::DeviceMemory m_ColorImageMemory = nullptr;
		vk::raii::ImageView m_ColorImageView = nullptr;
		vk::raii::Sampler m_ColorSampler = nullptr;
		
		void* m_ImGuiTextureID = nullptr;
	};
}

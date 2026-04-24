#pragma once

#include "Engine/Renderer/RHI/Framebuffer.h"

#include <vulkan/vulkan_raii.hpp>

namespace Engine
{
	class VulkanDevice;

	class VulkanFramebuffer final : public Framebuffer
	{
	public:
		VulkanFramebuffer(VulkanDevice& device, const FramebufferSpecification& spec);
		~VulkanFramebuffer() override;

		void Resize(uint32_t width, uint32_t height) override;
		void* GetImGuiTextureID() override;

		uint32_t GetWidth()  const override { return m_FramebufferSpec.Width; }
		uint32_t GetHeight() const override { return m_FramebufferSpec.Height; }
		bool HasColorAttachment() const override { return m_FramebufferSpec.ColorFormat != TextureFormat::Undefined; }
		bool HasDepthAttachment() const override { return m_FramebufferSpec.DepthFormat != TextureFormat::Undefined; }

		// ---- Color (resolve target / sampled image) ----
		vk::Image     GetImage()     const { return *m_ColorImage; }
		vk::ImageView GetImageView() const { return *m_ColorImageView; }
		vk::Sampler   GetSampler()   const { return *m_ColorSampler; }
		vk::Extent2D  GetExtent()    const { return vk::Extent2D(m_FramebufferSpec.Width, m_FramebufferSpec.Height); }

		vk::ImageLayout GetCurrentLayout() const { return m_CurrentColorLayout; }
		void            SetCurrentLayout(vk::ImageLayout layout) { m_CurrentColorLayout = layout; }

		// ---- Depth ----
		bool          HasDepth()          const { return *m_DepthImage != VK_NULL_HANDLE; }
		vk::Image     GetDepthImage()     const { return *m_DepthImage; }
		vk::ImageView GetDepthImageView() const { return *m_DepthImageView; }
		vk::Sampler   GetDepthSampler()   const { return *m_DepthSampler; }
		vk::ImageLayout GetCurrentDepthLayout() const { return m_CurrentDepthLayout; }
		void            SetCurrentDepthLayout(vk::ImageLayout layout) { m_CurrentDepthLayout = layout; }

		// ---- MSAA ----
		bool                    HasMSAA()        const { return m_MSAASamples != vk::SampleCountFlagBits::e1; }
		vk::SampleCountFlagBits GetMSAASamples() const { return m_MSAASamples; }
		vk::Image               GetMSAAImage()   const { return *m_MSAAColorImage; }

		// ---- Dynamic rendering info (used by BeginPass) ----
		vk::RenderingInfo BuildRenderingInfo(const glm::vec4& clearColor) const;

	private:
		void CreateColorAttachment();
		void CreateMSAAColorAttachment();
		void CreateDepthAttachment();
		void CreateColorSampler();
		void CreateDepthSampler();
		void ReleaseResources();
		void ReleaseImGuiTexture();

		VulkanDevice&            m_Device;
		FramebufferSpecification m_FramebufferSpec;

		// Resolved / sampled color image
		vk::raii::Image        m_ColorImage        = nullptr;
		vk::raii::DeviceMemory m_ColorImageMemory  = nullptr;
		vk::raii::ImageView    m_ColorImageView    = nullptr;
		vk::raii::Sampler      m_ColorSampler      = nullptr;
		vk::ImageLayout        m_CurrentColorLayout = vk::ImageLayout::eUndefined;

		// MSAA render target (only valid when SampleCount > 1)
		vk::SampleCountFlagBits m_MSAASamples         = vk::SampleCountFlagBits::e1;
		vk::raii::Image         m_MSAAColorImage       = nullptr;
		vk::raii::DeviceMemory  m_MSAAColorImageMemory = nullptr;
		vk::raii::ImageView     m_MSAAColorImageView   = nullptr;

		// Depth attachment (only valid when DepthFormat != Undefined)
		vk::raii::Image        m_DepthImage        = nullptr;
		vk::raii::DeviceMemory m_DepthImageMemory  = nullptr;
		vk::raii::ImageView    m_DepthImageView    = nullptr;
		vk::raii::Sampler      m_DepthSampler      = nullptr;
		vk::ImageLayout        m_CurrentDepthLayout = vk::ImageLayout::eUndefined;

		void* m_ImGuiTextureID = nullptr;
	};
}

#include "kepch.h"

#include "VulkanFramebuffer.h"
#include "VulkanContext.h"
#include "Engine/Core/Application.h"

#include "volk.h"

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

namespace Kairos
{
	static const uint32_t s_MaxFramebufferSize = 8192;

	namespace Utils
	{
		static VkImageType TextureTarget(bool multisampled)
		{
			return multisampled ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_2D;
		}

		static void CreateTextures(bool multisampled, uint32_t* outID, uint32_t count)
		{
			
		}

		static void BindTexture(bool multisampled, uint32_t id)
		{
			
		}

		static bool IsDepthFormat(FramebufferTextureFormat format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8:  return true;
			}

			return false;
		}

		static VkFormat KairosFBTextureFormatToGL(FramebufferTextureFormat format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::RGBA8:       return VK_FORMAT_R8G8B8A8_UINT;
			case FramebufferTextureFormat::RED_INTEGER: return VK_FORMAT_R8_UINT;
			}

			KE_CORE_ASSERT(false);
			return VK_FORMAT_MAX_ENUM;
		}

	}

	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& spec) : m_Specification(spec)
	{
		for (auto format : m_Specification.Attachments.Attachments)
		{
			if (Utils::IsDepthFormat(format.TextureFormat))
				m_DepthAttachmentSpecification = format;
			else
				m_ColorAttachmentSpecifications.emplace_back(format);
		}

		Invalidate();
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
		
	}

	void VulkanFramebuffer::Invalidate()
	{
		
	}

	void VulkanFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
		{
			KE_CORE_WARN("Attempted to rezize framebuffer to {0}, {1}", width, height);
			return;
		}

		m_Specification.Width = width;
		m_Specification.Height = height;

		Invalidate();
	}

	int VulkanFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
	{
		return 0;
	}

	void VulkanFramebuffer::ClearAttachment(uint32_t attachmentIndex, int value)
	{
		
	}

	uint32_t VulkanFramebuffer::GetColorAttachmentRendererID(uint32_t index) const
	{
		Application& app = Application::Get();
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();


		return 0;
	}
}
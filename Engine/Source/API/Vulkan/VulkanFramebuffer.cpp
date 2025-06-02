#include "kepch.h"

#include "VulkanFramebuffer.h"
#include "VulkanContext.h"

#include "Engine/Core/Application.h"

namespace Kairos
{
	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& spec) : m_Specification(spec)
	{
		for (auto spec : m_Specification.Attachments.Attachments)
		{
			//if (!Utils::IsDepthFormat(spec.TextureFormat))
				//m_ColorAttachmentSpecifications.emplace_back(spec);
			//else
				//m_DepthAttachmentSpecification = spec;
		}

		Invalidate();

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		//vctx->GetVkContext().m_Swapchain.CreateSwapchain(vctx, spec.Width, spec.Height);
		//vctx->GetVkContext().m_Swapchain.CreateDescriptorPool(vctx);
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
		
	}

	void VulkanFramebuffer::Bind()
	{
		
	}

	void VulkanFramebuffer::Unbind()
	{
		
	}

	void VulkanFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		
	}

	int VulkanFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
	{
		return 0;
	}

	void VulkanFramebuffer::ClearAttachment(uint32_t attachmentIndex, int value)
	{
		
	}
}
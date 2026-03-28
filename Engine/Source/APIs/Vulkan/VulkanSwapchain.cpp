#include "kepch.h"
#include "VulkanSwapchain.h"
#include "VulkanUtils.h"

namespace Engine
{
	VulkanSwapchain::VulkanSwapchain(VulkanDevice& device, GLFWwindow* windowHandle) : m_VulkanDevice(device), m_Window(windowHandle)
	{
		vk::PhysicalDeviceProperties physicalDeviceProperties = device.GetPhysicalDevice().getProperties();
		vk::SampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

		if (counts & vk::SampleCountFlagBits::e64) { m_MSAA_Samples = vk::SampleCountFlagBits::e64; }
		else if (counts & vk::SampleCountFlagBits::e32) { m_MSAA_Samples = vk::SampleCountFlagBits::e32; }
		else if (counts & vk::SampleCountFlagBits::e16) { m_MSAA_Samples = vk::SampleCountFlagBits::e16; }
		else if (counts & vk::SampleCountFlagBits::e8) { m_MSAA_Samples = vk::SampleCountFlagBits::e8; }
		else if (counts & vk::SampleCountFlagBits::e4) { m_MSAA_Samples = vk::SampleCountFlagBits::e4; }
		else if (counts & vk::SampleCountFlagBits::e2) { m_MSAA_Samples = vk::SampleCountFlagBits::e2; }
		else { m_MSAA_Samples = vk::SampleCountFlagBits::e1; }
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
		Cleanup();
	}

	bool VulkanSwapchain::Create()
	{
		try
		{
			SwapChainSupportDetails swapChainSupport = VulkanUtils::QuerySwapChainSupport(m_VulkanDevice.GetPhysicalDevice(), m_VulkanDevice.GetSurface());

			vk::SurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
			vk::PresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
			vk::Extent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);
			
			uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
			if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
			{
				imageCount = swapChainSupport.capabilities.maxImageCount;
			}
			
			vk::SwapchainCreateInfoKHR createInfo;
			createInfo.surface = *m_VulkanDevice.GetSurface();
			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
			createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
			createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
			createInfo.presentMode = presentMode;
			createInfo.clipped = VK_TRUE;

			QueueFamilyIndices indices = m_VulkanDevice.GetQueueFamilyIndices();
			std::array<uint32_t, 2> queueFamilyIndicesLoc = {indices.graphicsFamily.value(), indices.presentFamily.value()};
			
			if (indices.graphicsFamily != indices.presentFamily)
			{
				createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
				createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndicesLoc.size());
				createInfo.pQueueFamilyIndices = queueFamilyIndicesLoc.data();
			}
			else
			{
				createInfo.imageSharingMode = vk::SharingMode::eExclusive;
			}
			
			m_SwapChain = vk::raii::SwapchainKHR(m_VulkanDevice.GetDevice(), createInfo);
			m_SwapChainImages = m_SwapChain.getImages();
			m_SwapChainImageLayouts.assign(m_SwapChainImages.size(), vk::ImageLayout::eUndefined);
			m_SwapChainImageFormat = surfaceFormat.format;
			m_SwapChainExtent = extent;
			
			return true;
		}
		catch (const exception& e)
		{
			LOG(LogLevel::Error, "Failed to create Swapchain: {}", e.what());
			return false;
		}
	}

	bool VulkanSwapchain::CreateImageViews()
	{
		try
		{
			m_SwapChainImageViews.clear();
			m_SwapChainImageViews.reserve(m_SwapChainImages.size());
			
			vk::ImageViewCreateInfo createInfo{};
			createInfo.viewType = vk::ImageViewType::e2D;
			createInfo.format = m_SwapChainImageFormat;
			createInfo.components.r = vk::ComponentSwizzle::eIdentity;
			createInfo.components.g = vk::ComponentSwizzle::eIdentity;
			createInfo.components.b = vk::ComponentSwizzle::eIdentity;
			createInfo.components.a = vk::ComponentSwizzle::eIdentity;
			createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			
			for (const auto& image : m_SwapChainImages)
			{
				createInfo.image = image;
				m_SwapChainImageViews.emplace_back(m_VulkanDevice.GetDevice(), createInfo);
			}
			
			return true;
		}
		catch (const exception& e)
		{
			LOG(LogLevel::Error, "Failed to create Image views: {}", e.what());
			return false;
		}
	}

	void VulkanSwapchain::Cleanup()
	{
		m_SwapChainImageViews.clear();
		m_SwapChain = vk::raii::SwapchainKHR(nullptr);
		m_ColorImageView = vk::raii::ImageView(nullptr);
		m_ColorImageMemory = vk::raii::DeviceMemory(nullptr);
		m_ColorImage = vk::raii::Image(nullptr);
		m_DepthImageView = vk::raii::ImageView(nullptr);
		m_DepthImageMemory = vk::raii::DeviceMemory(nullptr);
		m_DepthImage = vk::raii::Image(nullptr);
		m_ColorAttachments.clear();
		m_DepthImageFormat = vk::Format::eUndefined;
	}

	bool VulkanSwapchain::Recreate()
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(m_Window, &width, &height);

		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(m_Window, &width, &height);
			glfwWaitEvents();
		}
		
		m_VulkanDevice.WaitIdle();
		
		Cleanup();
		Create();
		CreateImageViews();
		CreateColorResources();
		CreateDepthResources();
		SetupDynamicRendering();
		
		return true;
	}

	void VulkanSwapchain::CreateColorResources()
	{
		VulkanUtils::CreateImage(m_VulkanDevice.GetDevice(), m_VulkanDevice.GetPhysicalDevice(),
			m_SwapChainExtent.width, m_SwapChainExtent.height, 1, m_MSAA_Samples,
			m_SwapChainImageFormat, vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal,
			m_ColorImage, m_ColorImageMemory);

		m_ColorImageView = VulkanUtils::CreateImageView(m_VulkanDevice.GetDevice(), m_ColorImage, m_SwapChainImageFormat, vk::ImageAspectFlagBits::eColor, 1);
	}

	void VulkanSwapchain::CreateDepthResources()
	{
		m_DepthImageFormat = FindDepthFormat();

		VulkanUtils::CreateImage(m_VulkanDevice.GetDevice(), m_VulkanDevice.GetPhysicalDevice(),
			m_SwapChainExtent.width, m_SwapChainExtent.height, 1, m_MSAA_Samples,
			m_DepthImageFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal,
			m_DepthImage, m_DepthImageMemory);

		m_DepthImageView = VulkanUtils::CreateImageView(m_VulkanDevice.GetDevice(), m_DepthImage, m_DepthImageFormat, vk::ImageAspectFlagBits::eDepth, 1);
	}

	bool VulkanSwapchain::SetupDynamicRendering()
	{
		try
		{
			m_ColorAttachments.clear();

			vk::ClearValue clearColor = vk::ClearColorValue(0.01f, 0.01f, 0.01f, 1.0f);
			vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
			
			vk::RenderingAttachmentInfo colorAttachment;
			colorAttachment.imageView = m_ColorImageView;
			colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
			colorAttachment.resolveMode = vk::ResolveModeFlagBits::eAverage;
			colorAttachment.resolveImageView = m_SwapChainImageViews[0];
			colorAttachment.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
			colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
			colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
			colorAttachment.clearValue = clearColor;
			m_ColorAttachments.push_back(colorAttachment);

			m_DepthAttachment.imageView = m_DepthImageView;
			m_DepthAttachment.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			m_DepthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
			m_DepthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
			m_DepthAttachment.clearValue = clearDepth;

			m_RenderingInfo.renderArea = vk::Rect2D(vk::Offset2D(0, 0), m_SwapChainExtent);
			m_RenderingInfo.layerCount = 1;
			m_RenderingInfo.colorAttachmentCount = static_cast<uint32_t>(m_ColorAttachments.size());
			m_RenderingInfo.pColorAttachments = m_ColorAttachments.data();
			m_RenderingInfo.pDepthAttachment = &m_DepthAttachment;
			
			return true;
		}
		catch (const exception& e)
		{
			LOG(LogLevel::Error, "Failed to setup dynamic rendering: {}", e.what());
			return false;
		}
	}

	const vk::RenderingInfo& VulkanSwapchain::GetRenderingInfo(uint32_t imageIndex)
	{
		m_ColorAttachments[0].resolveImageView = m_SwapChainImageViews[imageIndex];
		return m_RenderingInfo;
	}

	vk::SurfaceFormatKHR VulkanSwapchain::ChooseSwapSurfaceFormat(const vector<vk::SurfaceFormatKHR>& availableFormats) const
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	vk::PresentModeKHR VulkanSwapchain::ChooseSwapPresentMode(const vector<vk::PresentModeKHR>& availablePresentModes) const
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == vk::PresentModeKHR::eMailbox)
			{
				return availablePresentMode;
			}
		}

		return vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D VulkanSwapchain::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const
	{
		if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
		{
			return capabilities.currentExtent;
		}
		
		int width, height;
		glfwGetFramebufferSize(m_Window, &width, &height);

		return {
			std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
			std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
		};
	}

	vk::Format VulkanSwapchain::FindDepthFormat() const
	{
		return VulkanUtils::FindSupportedFormat(m_VulkanDevice.GetPhysicalDevice(),
			{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment);
	}
}

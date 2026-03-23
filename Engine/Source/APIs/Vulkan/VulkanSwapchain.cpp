#include "kepch.h"
#include "VulkanSwapchain.h"
#include "VulkanUtils.h"

namespace Engine
{
	VulkanSwapchain::VulkanSwapchain(VulkanDevice& device, GLFWwindow* windowHandle) : m_VulkanDevice(device), m_Window(windowHandle)
	{}

	VulkanSwapchain::~VulkanSwapchain()
	{
		Cleanup();
	}

	bool VulkanSwapchain::Create()
	{
		try
		{
			// Query swap chain support
			SwapChainSupportDetails swapChainSupport = VulkanUtils::QuerySwapChainSupport(m_VulkanDevice.GetPhysicalDevice(), m_VulkanDevice.GetSurface());

			// Choose swap surface format, present mode, and extent
			vk::SurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
			vk::PresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
			vk::Extent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);
			
			// Choose image count
			uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
			if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
				imageCount = swapChainSupport.capabilities.maxImageCount;
			
			// Create swap chain info
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
			createInfo.oldSwapchain = nullptr;

			// Find queue families
			QueueFamilyIndices indices = m_VulkanDevice.GetQueueFamilyIndices();
			std::array<uint32_t, 2> queueFamilyIndicesLoc = {indices.graphicsFamily.value(), indices.presentFamily.value()};
			
			// Set sharing mode
			if (indices.graphicsFamily != indices.presentFamily)
			{
				createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
				createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndicesLoc.size());
				createInfo.pQueueFamilyIndices = queueFamilyIndicesLoc.data();
			} else 
			{
				createInfo.imageSharingMode = vk::SharingMode::eExclusive;
				createInfo.queueFamilyIndexCount = 0;
				createInfo.pQueueFamilyIndices = nullptr;
			}
			
			// Create swap chain
			m_SwapChain = vk::raii::SwapchainKHR(m_VulkanDevice.GetDevice(), createInfo);

			// Get swap chain images
			m_SwapChainImages = m_SwapChain.getImages();

			// Swapchain images start in UNDEFINED layout; track per-image layout for correct barriers.
			m_SwapChainImageLayouts.assign(m_SwapChainImages.size(), vk::ImageLayout::eUndefined);

			// Store swap chain format and extent
			m_SwapChainImageFormat = surfaceFormat.format;
			m_SwapChainExtent = extent;
			
			return true;
			
		} catch (const exception& e)
		{
			cerr << "Failed to create swap chain: " << e.what() << "\n";
			return false;
		}
	}

	bool VulkanSwapchain::CreateImageViews()
	{
		try
		{
			// Resize image views vector
			m_SwapChainImageViews.clear();
			m_SwapChainImageViews.reserve(m_SwapChainImages.size());
			
			// Create image view info template (image will be set per iteration)
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
			
			// Create image view for each swap chain image
			for (const auto& image : m_SwapChainImages)
			{
				createInfo.image = image;
				m_SwapChainImageViews.emplace_back(m_VulkanDevice.GetDevice(), createInfo);
			}
			
			return true;
			
		} catch (const exception& e)
		{
			cerr << "Failed to create image views: " << e.what() << "\n";
			return false;
		}
	}

	void VulkanSwapchain::Cleanup()
	{
		m_SwapChainImageViews.clear();
		m_SwapChain = vk::raii::SwapchainKHR(nullptr);
	}

	bool VulkanSwapchain::Recreate()
	{
		m_VulkanDevice.WaitIdle();
		
		Cleanup();
		
		Create();
		CreateImageViews();
		
		return true;
	}

	vk::SurfaceFormatKHR VulkanSwapchain::ChooseSwapSurfaceFormat(const vector<vk::SurfaceFormatKHR>& availableFormats)
	{
		// Look for SRGB format
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
				return availableFormat;
			}
		}

		// If not found, return first available format
		return availableFormats[0];
	}

	vk::PresentModeKHR VulkanSwapchain::ChooseSwapPresentMode(const vector<vk::PresentModeKHR>& availablePresentModes)
	{
		// Look for mailbox mode (triple buffering)
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
				return availablePresentMode;
			}
		}

		// If not found, return FIFO mode (guaranteed to be available)
		return vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D VulkanSwapchain::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
			return capabilities.currentExtent;
		
		int width, height;
		glfwGetFramebufferSize(m_Window, &width, &height);

		return {
			std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
			std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height) };
	}
}

#include "kepch.h"
#include "VulkanSwapchain.h"
#include "APIs/Vulkan/VulkanUtils.h"

namespace Engine
{
	VulkanSwapchain::VulkanSwapchain(VulkanDevice& device, GLFWwindow* windowHandle)
		: m_VulkanDevice(device), m_Window(windowHandle)
	{
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
				imageCount = swapChainSupport.capabilities.maxImageCount;

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

		return true;
	}

	vk::SurfaceFormatKHR VulkanSwapchain::ChooseSwapSurfaceFormat(const vector<vk::SurfaceFormatKHR>& availableFormats) const
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
				return availableFormat;
		}
		return availableFormats[0];
	}

	vk::PresentModeKHR VulkanSwapchain::ChooseSwapPresentMode(const vector<vk::PresentModeKHR>& availablePresentModes) const
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == vk::PresentModeKHR::eMailbox)
				return availablePresentMode;
		}
		return vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D VulkanSwapchain::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const
	{
		if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
			return capabilities.currentExtent;

		int width, height;
		glfwGetFramebufferSize(m_Window, &width, &height);

		return {
			std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
			std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
		};
	}
}

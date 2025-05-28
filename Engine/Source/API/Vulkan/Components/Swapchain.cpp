#include "kepch.h"

#include "Swapchain.h"

#include "volk.h"

#include "API/Vulkan/VulkanContext.h"

namespace Kairos
{
    void Swapchain::RecreateSwapchain(VulkanContext* vctx, uint32_t width, uint32_t height)
    {
        for (auto frame : GetSwapchainInfo()->frames)
            vkDestroyImageView(vctx->GetVkContext().device, frame.ImageView, nullptr);

        vkDestroyDescriptorPool(vctx->GetVkContext().device, GetSwapchainInfo()->descriptorPool, nullptr);
        vkDestroySwapchainKHR(vctx->GetVkContext().device, GetSwapchainInfo()->swapchain, nullptr);

        CreateSwapchain(vctx, width, height);
    }

    void Swapchain::CreateSwapchain(VulkanContext* vctx, uint32_t width, uint32_t height)
	{
		VkSurfaceCapabilitiesKHR caps;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vctx->GetVkContext().physicalDevice, vctx->GetVkContext().surface, &caps);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(vctx->GetVkContext().physicalDevice, vctx->GetVkContext().surface, &formatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(vctx->GetVkContext().physicalDevice, vctx->GetVkContext().surface, &formatCount, formats.data());

        GetSwapchainInfo()->imageFormat = formats[0].format;
        
        VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = vctx->GetVkContext().surface,
        .minImageCount = std::clamp(3u, caps.minImageCount, caps.maxImageCount > 0 ? caps.maxImageCount : 8u),
        .imageFormat = GetSwapchainInfo()->imageFormat,
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = ChooseExtent(width, height, caps),
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = caps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = ChoosePresentMode(vctx->GetVkContext().physicalDevice, vctx->GetVkContext().surface),
        .clipped = VK_TRUE
        };

        VK_CHECK(vkCreateSwapchainKHR(vctx->GetVkContext().device, &createInfo, nullptr, &GetSwapchainInfo()->swapchain));

        vctx->GetDeviceDeletionQueue().push_back([this](VkDevice device)
            {
                if (GetSwapchainInfo()->swapchain  != VK_NULL_HANDLE)
                {
                    vkDestroySwapchainKHR(device, GetSwapchainInfo()->swapchain, nullptr);
                }
            });

        uint32_t imageCount;
        vkGetSwapchainImagesKHR(vctx->GetVkContext().device, GetSwapchainInfo()->swapchain, &imageCount, nullptr);
        GetSwapchainInfo()->images.resize(imageCount);
        vkGetSwapchainImagesKHR(vctx->GetVkContext().device, GetSwapchainInfo()->swapchain, &imageCount, GetSwapchainInfo()->images.data());

        for (uint32_t i = 0; i < GetSwapchainInfo()->images.size(); ++i)
            GetSwapchainInfo()->frames.push_back(Frame(GetSwapchainInfo()->images[i], vctx->GetVkContext().device, GetSwapchainInfo()->imageFormat, vctx->GetDeviceDeletionQueue()));

        VkDescriptorPoolSize poolSizes[] =
        {
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, imageCount * 2 }
        };

        VkDescriptorPoolCreateInfo poolInfo =
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = imageCount,
            .poolSizeCount = 1,
            .pPoolSizes = poolSizes
        };

        VK_CHECK(vkCreateDescriptorPool(vctx->GetVkContext().device, &poolInfo, nullptr, &GetSwapchainInfo()->descriptorPool));

        vctx->GetDeviceDeletionQueue().push_back([this](VkDevice device)
            {
                vkDestroyDescriptorPool(device, GetSwapchainInfo()->descriptorPool, nullptr);
            });
	}

    VkExtent2D Swapchain::ChooseExtent(uint32_t width, uint32_t height, VkSurfaceCapabilitiesKHR capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        }

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(
            actualExtent.width,
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width
        );

        actualExtent.height = std::clamp(
            actualExtent.height,
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height
        );

        return actualExtent;
    }

    VkPresentModeKHR Swapchain::ChoosePresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
    {
        // Available present modes
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

        // Preferred modes in order of priority
        const std::vector<VkPresentModeKHR> preferredModes = {
            VK_PRESENT_MODE_MAILBOX_KHR,    // Triple buffering (low latency)
            VK_PRESENT_MODE_FIFO_KHR,       // Standard vsync (always available)
            VK_PRESENT_MODE_IMMEDIATE_KHR,  // No vsync (tearing possible)
        };

        for (auto mode : preferredModes) {
            if (std::find(presentModes.begin(), presentModes.end(), mode) != presentModes.end()) {
                if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
                    return mode;
            }
        }

        KE_CORE_WARN("Falling back to FIFO present mode [Standard VSync]");
        return VK_PRESENT_MODE_FIFO_KHR;
    }
}
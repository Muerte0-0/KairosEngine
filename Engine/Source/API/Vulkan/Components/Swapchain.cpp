#include "kepch.h"

#include "Swapchain.h"

#include "volk.h"

#include "API/Vulkan/VulkanContext.h"
#include <GLFW/glfw3.h>

namespace Kairos
{
    void Swapchain::RecreateSwapchain(VulkanContext* vctx)
    {
        vkDeviceWaitIdle(vctx->GetVkContext().device);

        Destroy(vctx);

        int width, height;
        glfwGetWindowSize(vctx->GetWindowHandle(), &width, &height);

        CreateSwapchain(vctx, width, height);
    }

    void Swapchain::Destroy(VulkanContext* vctx)
    {
        while (m_DeletionQueue.size() > 0)
        {
            m_DeletionQueue.back()(vctx->GetVkContext().device);
            m_DeletionQueue.pop_back();
        }

        m_SwapchainInfo.images.clear();
        m_SwapchainInfo.frames.clear();
    }

    void Swapchain::CreateSwapchain(VulkanContext* vctx, uint32_t width, uint32_t height)
	{
        SurfaceDetails support = QuerySurfaceSupport(vctx->GetVkContext().physicalDevice, vctx->GetVkContext().surface);

        m_SwapchainInfo.imageFormat = ChooseSurfaceFormat(support.formats);
        VkPresentModeKHR presentMode = ChoosePresentMode(support.presentModes);
        m_SwapchainInfo.extent = ChooseExtent(width, height, support.capabilities);

        uint32_t imageCount = std::min(support.capabilities.maxImageCount, support.capabilities.minImageCount + 1);

        VkSwapchainCreateInfoKHR sci = {};

        sci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        sci.flags = VkSwapchainCreateFlagBitsKHR();
        sci.surface = vctx->GetVkContext().surface;
        sci.minImageCount = imageCount;
        sci.imageFormat = m_SwapchainInfo.imageFormat.format;
        sci.imageColorSpace = m_SwapchainInfo.imageFormat.colorSpace;
        sci.imageExtent = m_SwapchainInfo.extent;
        sci.imageArrayLayers = 1;
        sci.compositeAlpha = VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        sci.imageUsage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        sci.imageSharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
        sci.preTransform = support.capabilities.currentTransform;
        sci.presentMode = presentMode;
        sci.clipped = true;
        sci.oldSwapchain = VkSwapchainKHR(nullptr);

        VK_CHECK(vkCreateSwapchainKHR(vctx->GetVkContext().device, &sci, nullptr, &m_SwapchainInfo.swapchain));

        m_DeletionQueue.push_back([this](VkDevice device)
            {
                vkDestroySwapchainKHR(device, m_SwapchainInfo.swapchain, nullptr);
            });

        vkGetSwapchainImagesKHR(vctx->GetVkContext().device, m_SwapchainInfo.swapchain, &imageCount, nullptr);
        m_SwapchainInfo.images.resize(imageCount);
        vkGetSwapchainImagesKHR(vctx->GetVkContext().device, m_SwapchainInfo.swapchain, &imageCount, m_SwapchainInfo.images.data());

        for (uint32_t i = 0; i < m_SwapchainInfo.images.size(); ++i)
            m_SwapchainInfo.frames.push_back(Frame(m_SwapchainInfo.images[i], vctx->GetVkContext().device, m_SwapchainInfo.imageFormat.format, m_DeletionQueue));
	}

    void Swapchain::CreateDescriptorPool(VulkanContext* vctx)
    {
        VkDescriptorPoolSize pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * ((int)(sizeof(pool_sizes) / sizeof(*(pool_sizes))));
        pool_info.poolSizeCount = (uint32_t)((int)(sizeof(pool_sizes) / sizeof(*(pool_sizes))));
        pool_info.pPoolSizes = pool_sizes;

        VK_CHECK(vkCreateDescriptorPool(vctx->GetVkContext().device, &pool_info, nullptr, &m_SwapchainInfo.descriptorPool));

        vctx->GetDeviceDeletionQueue().push_back([this](VkDevice device)
            {
                vkDestroyDescriptorPool(device, m_SwapchainInfo.descriptorPool, nullptr);
            });
    }

    SurfaceDetails Swapchain::QuerySurfaceSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
    {
        SurfaceDetails support;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &support.capabilities);

        uint32_t surfaceFormatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, NULL);
        support.formats.resize(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, support.formats.data());

        uint32_t presentModesCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, NULL);
        support.presentModes.resize(presentModesCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, support.presentModes.data());

        return support;
    }

    VkExtent2D Swapchain::ChooseExtent(uint32_t width, uint32_t height, VkSurfaceCapabilitiesKHR capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
            return capabilities.currentExtent;

        VkExtent2D extent;

        extent.width = std::min(capabilities.maxImageExtent.width, std::max(capabilities.minImageExtent.width, width));
        extent.height = std::min(capabilities.maxImageExtent.height, std::max(capabilities.minImageExtent.height, height));

        return extent;
    }

    VkPresentModeKHR Swapchain::ChoosePresentMode(std::vector<VkPresentModeKHR> presentModes)
    {
        // Preferred modes in order of priority
        const std::vector<VkPresentModeKHR> preferredModes =
        {
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

    VkSurfaceFormatKHR Swapchain::ChooseSurfaceFormat(std::vector<VkSurfaceFormatKHR> formats)
    {
        for (VkSurfaceFormatKHR format : formats)
            if (format.format == VkFormat::VK_FORMAT_R8G8B8A8_UNORM && format.colorSpace == VkColorSpaceKHR::VK_COLORSPACE_SRGB_NONLINEAR_KHR)
                return format;

        return formats[0];
    }
}
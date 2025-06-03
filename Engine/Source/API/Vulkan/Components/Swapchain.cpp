#include "kepch.h"

#include "Swapchain.h"
#include "Image.h"

#include "volk.h"

#include "API/Vulkan/VulkanContext.h"
#include "Engine/Core/Application.h"

namespace Kairos
{
    void Swapchain::RecreateSwapchain(VkDevice logicalDevice, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, GLFWwindow* window)
    {
		KE_CORE_INFO("Recreating Swapchain...");

        int width, height;
        glfwGetWindowSize(window, &width, &height);

        vkDeviceWaitIdle(logicalDevice);

        Destroy(logicalDevice);

        CreateSwapchain(logicalDevice, physicalDevice, surface, width, height);
    }

    void Swapchain::RecreateSwapchain(VkDevice logicalDevice, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t width, uint32_t height)
    {
        KE_CORE_INFO("Recreating Swapchain...");

        vkDeviceWaitIdle(logicalDevice);

        Destroy(logicalDevice);

        CreateSwapchain(logicalDevice, physicalDevice, surface, width, height);
    }

    void Swapchain::Destroy(VkDevice logicalDevice)
    {
        while (m_DeletionQueue.size() > 0)
        {
            m_DeletionQueue.back()(logicalDevice);
            m_DeletionQueue.pop_back();
        }

        m_Info.Images.clear();
        m_Info.ImageViews.clear();
    }

    void Swapchain::CreateSwapchain(VkDevice logicalDevice, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t width, uint32_t height)
    {
        SurfaceDetails support = QuerySurfaceSupport(physicalDevice, surface);

        m_Info.ImageFormat = ChooseSurfaceFormat(support.Formats);
        VkPresentModeKHR presentMode = ChoosePresentMode(support.PresentModes);
        m_Info.Extent = ChooseExtent(width, height, support.Capabilities);

        m_Info.ImageCount = std::min(support.Capabilities.maxImageCount, support.Capabilities.minImageCount + 1);

        VkSwapchainCreateInfoKHR swapchainCreateInfo;
        swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.flags = VkSwapchainCreateFlagsKHR();
        swapchainCreateInfo.surface = surface;
        swapchainCreateInfo.minImageCount = m_Info.ImageCount;
        swapchainCreateInfo.imageFormat = m_Info.ImageFormat.format;
        swapchainCreateInfo.imageColorSpace = m_Info.ImageFormat.colorSpace;
        swapchainCreateInfo.imageExtent = m_Info.Extent;
        swapchainCreateInfo.imageArrayLayers = 1;
        swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.preTransform = support.Capabilities.currentTransform;
        swapchainCreateInfo.presentMode = presentMode;
        swapchainCreateInfo.clipped = true;
        swapchainCreateInfo.oldSwapchain = VkSwapchainKHR(nullptr);
		swapchainCreateInfo.pNext = nullptr;

        VK_CHECK(vkCreateSwapchainKHR(logicalDevice, &swapchainCreateInfo, nullptr, &m_Info.Swapchain));

		VkSwapchainKHR handle = m_Info.Swapchain;

        m_DeletionQueue.push_back([handle](VkDevice device)
            {
                vkDestroySwapchainKHR(device, handle, nullptr);
            });

        vkGetSwapchainImagesKHR(logicalDevice, m_Info.Swapchain, &m_Info.ImageCount, nullptr);
        m_Info.Images.resize(m_Info.ImageCount);
        vkGetSwapchainImagesKHR(logicalDevice, m_Info.Swapchain, &m_Info.ImageCount, m_Info.Images.data());

        for (uint32_t i = 0; i < m_Info.Images.size(); ++i)
        {
            VkImageView imageView = CreateImageView(logicalDevice, m_Info.Images[i], m_Info.ImageFormat.format);
            m_Info.ImageViews.push_back(imageView);

            m_DeletionQueue.push_back([imageView](VkDevice device)
            {
                vkDestroyImageView(device, imageView, nullptr);
            });
        }
    }

    void Swapchain::CreateDescriptorPool(VkDevice logicalDevice, std::deque<std::function<void(VkDevice)>>& deviceDeletionQueue)
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

        VK_CHECK(vkCreateDescriptorPool(logicalDevice, &pool_info, nullptr, &m_Info.DescriptorPool));

        VkDescriptorPool handle = m_Info.DescriptorPool;

        deviceDeletionQueue.push_back([handle](VkDevice device)
            {
                vkDestroyDescriptorPool(device, handle, nullptr);
            });
    }

    SurfaceDetails Swapchain::QuerySurfaceSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
    {
        SurfaceDetails support;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &support.Capabilities);

        uint32_t surfaceFormatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, NULL);
        support.Formats.resize(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, support.Formats.data());

        uint32_t presentModesCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, NULL);
        support.PresentModes.resize(presentModesCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, support.PresentModes.data());

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
                {
                    return mode;
					KE_CORE_INFO("Using Mailbox present mode [Triple Buffering]");
                }
            }
        }

        KE_CORE_WARN("Falling back to FIFO present mode [Standard VSync]");
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkSurfaceFormatKHR Swapchain::ChooseSurfaceFormat(std::vector<VkSurfaceFormatKHR> formats)
    {
        for (VkSurfaceFormatKHR format : formats)
            if (format.format == VK_FORMAT_R8G8B8A8_UNORM && format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
                return format;

        return formats[0];
    }
}
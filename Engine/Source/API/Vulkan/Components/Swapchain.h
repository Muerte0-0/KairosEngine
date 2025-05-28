#pragma once

#include "Frame.h"

namespace Kairos
{
	struct VkSwapchain
	{
		VkSwapchainKHR swapchain;
		std::vector<VkImage> images;
		std::vector<Frame> frames;
		VkFormat imageFormat;
		VkExtent2D extent;

		// Dynamic rendering
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
	};

	class Swapchain 
	{
	public:
		void RecreateSwapchain(class VulkanContext* vctx, uint32_t width, uint32_t height);

		VkSwapchain* GetSwapchainInfo() {return &m_SwapchainInfo;}

	private:
		VkSwapchain m_SwapchainInfo;

		void CreateSwapchain(class VulkanContext* vctx, uint32_t width, uint32_t height);
		VkExtent2D ChooseExtent(uint32_t width, uint32_t height, VkSurfaceCapabilitiesKHR capabilities);
		VkPresentModeKHR ChoosePresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	};

}
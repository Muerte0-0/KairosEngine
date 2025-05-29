#pragma once

#include "Frame.h"

namespace Kairos
{
	struct VkSwapchain
	{
		VkSwapchainKHR swapchain;
		std::vector<VkImage> images;
		std::vector<Frame> frames;
		VkSurfaceFormatKHR imageFormat;
		VkExtent2D extent;

		// Dynamic rendering
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
	};

	struct SurfaceDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class Swapchain 
	{
	public:
		void RecreateSwapchain(class VulkanContext* vctx);
		void CreateSwapchain(class VulkanContext* vctx, uint32_t width, uint32_t height);

		VkSwapchain& GetSwapchainInfo() {return m_SwapchainInfo;}

		void Destroy(VulkanContext* vctx);

	private:
		VkSwapchain m_SwapchainInfo;
		std::deque<std::function<void(VkDevice)>> m_DeletionQueue;

		SurfaceDetails QuerySurfaceSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
		VkExtent2D ChooseExtent(uint32_t width, uint32_t height, VkSurfaceCapabilitiesKHR capabilities);
		VkPresentModeKHR ChoosePresentMode(std::vector<VkPresentModeKHR> presentModes);
		VkSurfaceFormatKHR ChooseSurfaceFormat(std::vector<VkSurfaceFormatKHR> formats);
	};

}
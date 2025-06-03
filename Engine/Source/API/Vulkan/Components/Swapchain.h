#pragma once
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#include "GLFW/glfw3.h"

namespace Kairos
{
	struct SwapchainInfo
	{
		VkSwapchainKHR Swapchain;
		VkSurfaceFormatKHR ImageFormat;
		VkExtent2D Extent;

		uint32_t ImageCount;
		std::vector<VkImageView> ImageViews;
		std::vector<VkImage> Images;

		// Dynamic rendering
		VkDescriptorPool DescriptorPool;
		std::vector<VkDescriptorSet> DescriptorSets;
	};

	struct SurfaceDetails
	{
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};

	class Swapchain 
	{
	public:
		void RecreateSwapchain(VkDevice logicalDevice, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, GLFWwindow* window);

		void Destroy(VkDevice logicalDevice);

		void CreateSwapchain(VkDevice logicalDevice, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t width, uint32_t height);
		void CreateDescriptorPool(VkDevice logicalDevice, std::deque<std::function<void(VkDevice)>>& deviceDeletionQueue);

		SwapchainInfo& Info() {return m_Info;}

	private:
		SwapchainInfo m_Info;
		std::deque<std::function<void(VkDevice)>> m_DeletionQueue;

		SurfaceDetails QuerySurfaceSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
		VkExtent2D ChooseExtent(uint32_t width, uint32_t height, VkSurfaceCapabilitiesKHR capabilities);
		VkPresentModeKHR ChoosePresentMode(std::vector<VkPresentModeKHR> presentModes);
		VkSurfaceFormatKHR ChooseSurfaceFormat(std::vector<VkSurfaceFormatKHR> formats);
	};

}
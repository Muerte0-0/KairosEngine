#pragma once
#include <vulkan/vulkan_raii.hpp>

namespace Engine
{	
	/**
	 * @brief Structure for SwapChain support Details.
	*/
	struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		vector<vk::SurfaceFormatKHR> formats;
		vector<vk::PresentModeKHR> presentModes;
	};
	
	class VulkanUtils
	{
	public:
		
		/**
		 * @brief Find a memory type with the specified properties.
		 * @param physicalDevice The Physical Device
		 * @param typeFilter The type filter.
		 * @param properties The memory properties.
		 * @return The memory type index.
		*/
		static uint32_t FindMemoryType(const vk::raii::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);
		
		/**
		 * @brief Query swap chain support for a physical device.
		 * @param device The Physical Device.
		 * @param surface The Surface.
		 * @return The swap chain support details.
		*/
		static SwapChainSupportDetails QuerySwapChainSupport(const vk::raii::PhysicalDevice &device, const vk::raii::SurfaceKHR& surface);
	};
}

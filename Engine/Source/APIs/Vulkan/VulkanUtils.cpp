#include "kepch.h"
#include "VulkanUtils.h"

namespace Engine
{
	uint32_t VulkanUtils::FindMemoryType(const vk::raii::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties)
	{
		// Get memory properties
		vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

		// Find suitable memory type
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;

		throw runtime_error("Failed to find suitable memory type");
	}

	SwapChainSupportDetails VulkanUtils::QuerySwapChainSupport(const vk::raii::PhysicalDevice& device, const vk::raii::SurfaceKHR& surface)
	{
		SwapChainSupportDetails details;

		// Get surface capabilities
		details.capabilities = device.getSurfaceCapabilitiesKHR(*surface);

		// Get surface formats
		details.formats = device.getSurfaceFormatsKHR(*surface);

		// Get present modes
		details.presentModes = device.getSurfacePresentModesKHR(*surface);

		return details;
	}
}

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

	vk::Format VulkanUtils::FindSupportedFormat(const vk::raii::PhysicalDevice& physicalDevice, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
		vk::FormatFeatureFlags features)
	{
		auto formatIt = std::ranges::find_if(candidates, [&](auto const format) 
			{
				vk::FormatProperties props = physicalDevice.getFormatProperties(format);
				return (((tiling == vk::ImageTiling::eLinear) && ((props.linearTilingFeatures & features) == features)) ||
					((tiling == vk::ImageTiling::eOptimal) && ((props.optimalTilingFeatures & features) == features)));
			});

		if (formatIt == candidates.end())
			throw std::runtime_error("failed to find supported format!");

		return *formatIt;
	}

	vk::raii::ImageView VulkanUtils::CreateImageView(const vk::raii::Device& device, vk::raii::Image& image, vk::Format format, 
													 vk::ImageAspectFlags aspectFlags, uint32_t mipLevels)
	{
		vk::ImageViewCreateInfo viewInfo;
		viewInfo.image = image;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = format;
		viewInfo.subresourceRange = {aspectFlags, 0, mipLevels, 0, 1};
		return vk::raii::ImageView(device, viewInfo);
	}

	void VulkanUtils::CreateImage(const vk::raii::Device& device, const vk::raii::PhysicalDevice& physicalDevice, uint32_t width, uint32_t height, uint32_t mipLevels,
		vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags properties, vk::raii::Image& image, vk::raii::DeviceMemory& imageMemory)
	{
		vk::ImageCreateInfo imageInfo;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.format = format;
		imageInfo.extent = vk::Extent3D(width, height, 1 );
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = numSamples;
		imageInfo.tiling = tiling;
		imageInfo.usage = usage;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;

		image = vk::raii::Image(device, imageInfo);

		vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
		vk::MemoryAllocateInfo allocInfo(memRequirements.size, FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties));
		imageMemory = vk::raii::DeviceMemory(device, allocInfo);
		image.bindMemory(imageMemory, 0);
	}

	Scope<vk::raii::CommandBuffer> VulkanUtils::BeginSingleTimeCommands(const vk::raii::Device& device, const vk::raii::CommandPool& commandPool)
	{
		vk::CommandBufferAllocateInfo allocInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1);
		std::unique_ptr<vk::raii::CommandBuffer> commandBuffer = std::make_unique<vk::raii::CommandBuffer>(std::move(vk::raii::CommandBuffers(device, allocInfo).front()));

		vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		commandBuffer->begin(beginInfo);

		return commandBuffer;
	}

	void VulkanUtils::EndSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer, const vk::raii::Queue& queue)
	{
		commandBuffer.end();

		vk::SubmitInfo submitInfo;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &*commandBuffer;

		queue.submit(submitInfo, nullptr);
		queue.waitIdle();
	}
}

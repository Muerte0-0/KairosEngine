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

	void VulkanUtils::TransitionImageLayout(vk::CommandBuffer commandBuffer, vk::Image image, vk::ImageLayout old_layout, vk::ImageLayout new_layout,
		vk::AccessFlags2 src_access_mask, vk::AccessFlags2 dst_access_mask, vk::PipelineStageFlags2 src_stage_mask,
		vk::PipelineStageFlags2 dst_stage_mask, vk::ImageAspectFlags image_aspect_flags)
	{
		vk::ImageMemoryBarrier2 barrier;

		barrier.srcStageMask = src_stage_mask;
		barrier.srcAccessMask = src_access_mask;
		barrier.dstStageMask = dst_stage_mask;
		barrier.dstAccessMask = dst_access_mask;
		barrier.oldLayout = old_layout;
		barrier.newLayout = new_layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		
		vk::ImageSubresourceRange subresRange;
		subresRange.aspectMask = image_aspect_flags;
		subresRange.baseMipLevel = 0;
		subresRange.levelCount = 1;
		subresRange.baseArrayLayer = 0;
		subresRange.layerCount = 1;

		barrier.subresourceRange = subresRange;

		vk::DependencyInfo dependency_info;
		dependency_info.dependencyFlags = {};
		dependency_info.imageMemoryBarrierCount = 1;
		dependency_info.pImageMemoryBarriers = &barrier;

		commandBuffer.pipelineBarrier2(dependency_info);
	}
	
	vk::Format VulkanUtils::ToVulkanFormat(TextureFormat format)
	{
		switch (format)
		{
			case TextureFormat::RGBA8_UNorm:        return vk::Format::eR8G8B8A8Unorm;
			case TextureFormat::BGRA8_UNorm:        return vk::Format::eB8G8R8A8Unorm;

			case TextureFormat::BGRA8_SRGB:         return vk::Format::eB8G8R8A8Srgb;

			case TextureFormat::RGBA32_Float:       return vk::Format::eR32G32B32A32Sfloat;

			case TextureFormat::Depth24Stencil8:    return vk::Format::eD24UnormS8Uint;
			case TextureFormat::Depth32_Float:      return vk::Format::eD32Sfloat;

			case TextureFormat::R8_UNorm:           return vk::Format::eR8Unorm;
			case TextureFormat::RG8_UNorm:          return vk::Format::eR8G8Unorm;

			case TextureFormat::Undefined:          
			default:                                return vk::Format::eUndefined;
		}
	}
	
	vk::SampleCountFlagBits VulkanUtils::ToVulkanSampleCount(SampleCountBits samples)
	{
		switch (samples)
		{
			case SampleCountBits::s1:	return vk::SampleCountFlagBits::e1;
			case SampleCountBits::s2:	return vk::SampleCountFlagBits::e2;
			case SampleCountBits::s4:	return vk::SampleCountFlagBits::e4;
			case SampleCountBits::s8:	return vk::SampleCountFlagBits::e8;
			case SampleCountBits::s16:	return vk::SampleCountFlagBits::e16;
			default:					return vk::SampleCountFlagBits::e1;
		}
	}
	
	TextureFormat VulkanUtils::ToTextureFormat(vk::Format format)
	{
		switch (format)
		{
			case vk::Format::eR8G8B8A8Unorm:        return TextureFormat::RGBA8_UNorm;
			case vk::Format::eB8G8R8A8Unorm:        return TextureFormat::BGRA8_UNorm;

			case vk::Format::eR8G8B8A8Srgb:         return TextureFormat::RGBA8_SRGB;
			case vk::Format::eB8G8R8A8Srgb:         return TextureFormat::BGRA8_SRGB;

			case vk::Format::eR16G16B16A16Sfloat:   return TextureFormat::RGBA16_Float;
			case vk::Format::eR32G32B32A32Sfloat:   return TextureFormat::RGBA32_Float;

			case vk::Format::eD24UnormS8Uint:       return TextureFormat::Depth24Stencil8;
			case vk::Format::eD32Sfloat:            return TextureFormat::Depth32_Float;

			case vk::Format::eR8Unorm:              return TextureFormat::R8_UNorm;
			case vk::Format::eR8G8Unorm:            return TextureFormat::RG8_UNorm;

			case vk::Format::eUndefined:
			default:                                return TextureFormat::Undefined;
		}
	}
	
	SampleCountBits VulkanUtils::ToSampleCountBits(vk::SampleCountFlagBits samples)
	{
		switch (samples)
		{
			case vk::SampleCountFlagBits::e1:	return SampleCountBits::s1;
			case vk::SampleCountFlagBits::e2:	return SampleCountBits::s2;
			case vk::SampleCountFlagBits::e4:	return SampleCountBits::s4;
			case vk::SampleCountFlagBits::e8:	return SampleCountBits::s8;
			case vk::SampleCountFlagBits::e16:	return SampleCountBits::s16;
			default:					return SampleCountBits::s1;
		}
	}
}

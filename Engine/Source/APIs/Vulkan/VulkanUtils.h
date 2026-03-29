#pragma once
#include <vulkan/vulkan_raii.hpp>

#include "Engine/Renderer/RendererUtils.h"

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

		static vk::Format FindSupportedFormat(const vk::raii::PhysicalDevice& physicalDevice,
			const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
		
		/**
		 * @brief Helper Function to Create an Image View
		 * @param device The Logical Device
		 * @param image The Image used to Create the Image View
		 * @param format The Format
		 * @param aspectFlags The Aspect Flags
		 * @param mipLevels Mip Map Levels 
		 * @return The Image View
		*/
		static vk::raii::ImageView CreateImageView(const vk::raii::Device& device, vk::raii::Image& image, vk::Format format,
			vk::ImageAspectFlags aspectFlags, uint32_t mipLevels);
		
		static void CreateImage(const vk::raii::Device& device, const vk::raii::PhysicalDevice& physicalDevice, uint32_t width, uint32_t height, uint32_t mipLevels, 
			vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, 
			vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Image& image, vk::raii::DeviceMemory& imageMemory);
		
		static Scope<vk::raii::CommandBuffer> BeginSingleTimeCommands(const vk::raii::Device& device, const vk::raii::CommandPool& commandPool);
		static void EndSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer, const vk::raii::Queue& queue);
		
		static void TransitionImageLayout(
			vk::CommandBuffer			commandBuffer, 
			vk::Image					image,
			vk::ImageLayout				old_layout,
			vk::ImageLayout				new_layout,
			vk::AccessFlags2			src_access_mask,
			vk::AccessFlags2			dst_access_mask,
			vk::PipelineStageFlags2		src_stage_mask,
			vk::PipelineStageFlags2		dst_stage_mask,
			vk::ImageAspectFlags		image_aspect_flags);
		
		static vk::Format ToVulkanFormat(TextureFormat format);
		static vk::SampleCountFlagBits ToVulkanSampleCount(SampleCountBits samples);
		static TextureFormat ToTextureFormat(vk::Format format);
		static SampleCountBits ToSampleCountBits(vk::SampleCountFlagBits samples);
	};
}

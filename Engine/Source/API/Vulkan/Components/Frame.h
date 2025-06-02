#pragma once
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#include "Image.h"
#include "Swapchain.h"

namespace Kairos
{
	class Frame
	{
	public:
		Frame(Swapchain& swapchainRef, VkDevice device, std::vector<vk::ShaderEXT>& shaders, VkCommandBuffer commandBuffer, std::deque<std::function<void(VkDevice)>>& deletionQueue);

		void RecordCommandBuffer(uint32_t imageIndex);

		void SetCommandBuffer(VkCommandBuffer newCommandBuffer, std::vector<VkShaderEXT>& shaders, VkExtent2D frameSize);

		VkCommandBuffer CommandBuffer;

		Swapchain& SwapchainRef;
		std::vector<VkShaderEXT>& Shaders;

		VkSemaphore ImageAquiredSemaphore;
		VkSemaphore RenderFinishedSemaphore;

		VkFence RenderFinishedFence;

	private:
		void BuildRenderingInfo();
		void BuildColorAttachment(uint32_t imageIndex);
		void AnnoyingBoilerplateThatDynamicRenderingWasMeantToSpareUs();

		VkRenderingInfoKHR RenderingInfo = {};
		VkRenderingAttachmentInfoKHR ColorAttachment = {};

	};
}
#pragma once

#include <vulkan/vulkan.h>

#include "Image.h"
#include "Swapchain.h"

namespace Kairos
{
	class Frame
	{
	public:
		Frame(Swapchain& swapchainRef, VkDevice logicalDevice, VkCommandBuffer commandBuffer, std::deque<std::function<void(VkDevice)>>& deletionQueue);

		void RecordCommandBuffer(uint32_t imageIndex, const Ref<class VertexArray>& vertexArray);

		VkCommandBuffer CommandBuffer;

		Swapchain& SwapchainRef;

		VkSemaphore ImageAquiredSemaphore;
		VkSemaphore RenderFinishedSemaphore;

		VkFence RenderFinishedFence;

	private:
		void BuildRenderingInfo();
		void BuildColorAttachment(uint32_t imageIndex);

		VkRenderingInfoKHR RenderingInfo = {};
		VkRenderingAttachmentInfoKHR ColorAttachment = {};

	};
}
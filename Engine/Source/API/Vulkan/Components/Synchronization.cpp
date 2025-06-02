#include "kepch.h"
#include "Synchronization.h"

#include "volk.h"

namespace Kairos
{
	VkSemaphore MakeSemaphore(VkDevice logicalDevice, std::deque<std::function<void(VkDevice)>>& deviceDeletionQueue)
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo;
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.flags = 0;
		semaphoreCreateInfo.pNext = nullptr;

		VkSemaphore semaphore;
		vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &semaphore);

		deviceDeletionQueue.push_back([semaphore](VkDevice device)
			{
				vkDestroySemaphore(device, semaphore, nullptr);
			});

		return semaphore;
	}

	VkFence MakeFence(VkDevice logicalDevice, std::deque<std::function<void(VkDevice)>>& deviceDeletionQueue)
	{
		VkFenceCreateInfo fenceCreateInfo;
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		fenceCreateInfo.pNext = nullptr;

		VkFence fence;
		vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &fence);

		deviceDeletionQueue.push_back([fence](VkDevice device)
			{
				vkDestroyFence(device, fence, nullptr);
			});

		return fence;
	}

}
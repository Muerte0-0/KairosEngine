#include "kepch.h"

#include "Command.h"
#include "API/Vulkan/VulkanContext.h"

#include "volk.h"

namespace Kairos
{
	VkCommandPool CreateCommandPool(VkDevice logicalDevice, uint32_t queueFamilyIndex, std::deque<std::function<void(VkDevice)>>& deviceDeletionQueue)
	{
		VkCommandPoolCreateInfo commandPoolCreateInfo = {};
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;

		VkCommandPool pool = nullptr;

		VK_CHECK(vkCreateCommandPool(logicalDevice, &commandPoolCreateInfo, nullptr, &pool));
		deviceDeletionQueue.push_back([pool](VkDevice device)
			{
				vkDestroyCommandPool(device, pool, nullptr);
				KE_CORE_INFO("Destroyed Command Pool!");
			});

		return pool;
	}

	VkCommandBuffer AllocateCommandBuffer(VkDevice logicalDevice, VkCommandPool commandPool)
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

		return commandBuffer;
	}

	VkCommandBuffer BeginSingleTimeCommands(VkDevice logicalDevice, VkCommandPool commandPool)
	{
		VkCommandBuffer commandBuffer = AllocateCommandBuffer(logicalDevice, commandPool);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pNext = nullptr;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void EndSingleTimeCommands(VkDevice logicalDevice, VkCommandBuffer commandBuffer, VkCommandPool commandPool, VkQueue graphicsQueue)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, nullptr);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
	}
}
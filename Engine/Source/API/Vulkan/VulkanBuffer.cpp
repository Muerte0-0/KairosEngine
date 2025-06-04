#include "kepch.h"
#include "VulkanBuffer.h"

#include "VulkanContext.h"
#include "Engine/Core/Application.h"

#include "Components/Command.h"

#include "volk.h"

namespace Kairos
{
	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			// Check if the memory type is suitable and has the required properties
			if ((typeFilter & (1 << i)) &&
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		KE_CORE_ERROR("Failed to Find Suitable Memory Type!");
		return -1;
	}

	void CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK(vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer));

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(logicalDevice, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

		VK_CHECK(vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &bufferMemory));

		vkBindBufferMemory(logicalDevice, buffer, bufferMemory, 0);
	}

	void CopyBuffer(VkDevice logicalDevice, VkCommandPool commandPool, VkQueue queue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands(logicalDevice, commandPool);

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		EndSingleTimeCommands(logicalDevice, commandBuffer, commandPool, queue);
	}

	VulkanVertexBuffer::VulkanVertexBuffer(float* vertices, uint32_t size)
	{
		Application& app = Application::Get();
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		CreateBuffer(vctx->GetVkContext().PhysicalDevice, vctx->GetVkContext().LogicalDevice, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_VertexBuffer, m_VertexBufferMemory);

		void* data;
		vkMapMemory(vctx->GetVkContext().LogicalDevice, m_VertexBufferMemory, 0, size, 0, &data);
		memcpy(data, vertices, size);
		vkUnmapMemory(vctx->GetVkContext().LogicalDevice, m_VertexBufferMemory);
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		VkDevice logicalDevice = volkGetLoadedDevice();
		vkDestroyBuffer(logicalDevice, m_VertexBuffer, nullptr);
		vkFreeMemory(logicalDevice, m_VertexBufferMemory, nullptr);
	}

	VulkanIndexBuffer::VulkanIndexBuffer(uint32_t* indices, uint32_t count)
	{
		VkDeviceSize size = count * sizeof(uint32_t);
		m_Count = size;

		Application& app = Application::Get();
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		CreateBuffer(vctx->GetVkContext().PhysicalDevice, vctx->GetVkContext().LogicalDevice, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer, m_IndexBufferMemory);

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(vctx->GetVkContext().PhysicalDevice, vctx->GetVkContext().LogicalDevice, size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(vctx->GetVkContext().LogicalDevice, stagingBufferMemory, 0, size, 0, &data);
		memcpy(data, indices, size);
		vkUnmapMemory(vctx->GetVkContext().LogicalDevice, stagingBufferMemory);

		CopyBuffer(vctx->GetVkContext().LogicalDevice, vctx->GetVkContext().CommandPool, vctx->GetVkContext().GraphicsQueue, stagingBuffer, m_IndexBuffer, size);

		vkDestroyBuffer(vctx->GetVkContext().LogicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(vctx->GetVkContext().LogicalDevice, stagingBufferMemory, nullptr);
	}

	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		VkDevice logicalDevice = volkGetLoadedDevice();

		vkDestroyBuffer(logicalDevice, m_IndexBuffer, nullptr);
		vkFreeMemory(logicalDevice, m_IndexBufferMemory, nullptr);
	}
}
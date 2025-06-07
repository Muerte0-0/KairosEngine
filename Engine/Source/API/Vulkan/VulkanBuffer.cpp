#include "kepch.h"
#include "VulkanBuffer.h"

#include "VulkanUtils.h"

#include "VulkanContext.h"
#include "Engine/Core/Application.h"

#include "Components/Command.h"

#include "volk.h"

namespace Kairos
{
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

	VulkanIndexBuffer::VulkanIndexBuffer(uint32_t* indices, uint32_t count) : m_Count(count)
	{
		VkDeviceSize size = count * sizeof(uint32_t);

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
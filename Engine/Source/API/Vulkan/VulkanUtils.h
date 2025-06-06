#pragma once
#include <vulkan/vulkan.h>

namespace Kairos
{
#define VK_CHECK(result) \
    if (result != VK_SUCCESS) { \
        KE_CORE_ERROR("Vulkan error: {}", (char*)result); \
        abort(); \
    }

	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void CopyBuffer(VkDevice logicalDevice, VkCommandPool commandPool, VkQueue queue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	VkVertexInputBindingDescription2EXT GetBindingDescription(const class BufferLayout& layout);

	std::vector<VkVertexInputAttributeDescription2EXT> GetAttributeDescriptions(const class BufferLayout& layout);

	void AnnoyingBoilerplateThatDynamicRenderingWasMeantToSpareUs(VkCommandBuffer commandBuffer, const class BufferLayout& layout, VkExtent2D extent);
}
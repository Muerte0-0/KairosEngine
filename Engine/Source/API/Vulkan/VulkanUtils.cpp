#include "kepch.h"
#include "VulkanUtils.h"

#include "Engine/Renderer/Buffer.h"

#include "Components/Command.h"

#include "volk.h"

namespace Kairos
{
	static VkFormat ShaderDataTypeToVulkanBaseType(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:    return VK_FORMAT_R32_SFLOAT;
		case ShaderDataType::Float2:   return VK_FORMAT_R32G32_SFLOAT;
		case ShaderDataType::Float3:   return VK_FORMAT_R32G32B32_SFLOAT;
		case ShaderDataType::Float4:   return VK_FORMAT_R32G32B32A32_SFLOAT;
		case ShaderDataType::Mat3:     return VK_FORMAT_R32G32B32_SFLOAT; // Typically handled as array of vec3
		case ShaderDataType::Mat4:     return VK_FORMAT_R32G32B32A32_SFLOAT; // Typically handled as array of vec4
		case ShaderDataType::Int:      return VK_FORMAT_R32_SINT;
		case ShaderDataType::Int2:     return VK_FORMAT_R32G32_SINT;
		case ShaderDataType::Int3:     return VK_FORMAT_R32G32B32_SINT;
		case ShaderDataType::Int4:     return VK_FORMAT_R32G32B32A32_SINT;
		case ShaderDataType::Bool:     return VK_FORMAT_R8_UINT;
		}

		KE_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return VK_FORMAT_MAX_ENUM;
	}

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

		EndSingleTimeCommands(logicalDevice, commandPool, commandBuffer, queue);
	}

	VkVertexInputBindingDescription2EXT GetBindingDescription(const BufferLayout& layout)
	{
		VkVertexInputBindingDescription2EXT bindingDescription = {};

		bindingDescription.sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
		bindingDescription.binding = 0;
		bindingDescription.stride = layout.GetStride();
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Each vertex has its own data
		bindingDescription.divisor = 1; // Instancing
		bindingDescription.pNext = nullptr; // No additional data

		return bindingDescription;
	}

	std::vector<VkVertexInputAttributeDescription2EXT> GetAttributeDescriptions(const BufferLayout& layout)
	{
		std::vector<VkVertexInputAttributeDescription2EXT> attributeDescriptions = {};

		int locationIndex = 0; // Location index for the shader attributes

		for (auto element : layout.GetElements())
		{
			VkVertexInputAttributeDescription2EXT desc = {};
			desc.sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
			desc.binding = 0;
			desc.location = locationIndex; // Location in the shader
			desc.format = ShaderDataTypeToVulkanBaseType(element.Type); // Format of the attribute (e.g., VK_FORMAT_R32G32B32_SFLOAT)
			desc.offset = element.Offset; // Offset in bytes from the start of the vertex data

			attributeDescriptions.push_back(desc);
			locationIndex++;
		}

		return attributeDescriptions;
	}

	void AnnoyingBoilerplateThatDynamicRenderingWasMeantToSpareUs(VkCommandBuffer commandBuffer, const BufferLayout& layout, VkExtent2D extent)
	{
		VkVertexInputBindingDescription2EXT bindingDescription = GetBindingDescription(layout);
		std::vector<VkVertexInputAttributeDescription2EXT> attributeDescriptions = GetAttributeDescriptions(layout);

		vkCmdSetVertexInputEXT(commandBuffer, 1, &bindingDescription, attributeDescriptions.size(), attributeDescriptions.data());

		if (extent.width != 0 && extent.height != 0)
		{
			VkViewport viewport = VkViewport
			{
				.x = 0.0f, .y = 0.0f,
				.width = (float)extent.width,
				.height = (float)extent.height,
				.minDepth = 0.0f, .maxDepth = 1.0f
			};

			vkCmdSetViewportWithCountEXT(commandBuffer, 1, &viewport);
		}

		VkRect2D scissor = VkRect2D({ 0,0 }, extent);
		vkCmdSetScissorWithCountEXT(commandBuffer, 1, &scissor);

		vkCmdSetRasterizerDiscardEnableEXT(commandBuffer, VK_FALSE);
		vkCmdSetPolygonModeEXT(commandBuffer, VK_POLYGON_MODE_FILL);
		vkCmdSetRasterizationSamplesEXT(commandBuffer, VK_SAMPLE_COUNT_1_BIT);

		uint32_t sampleMask = 1;
		vkCmdSetSampleMaskEXT(commandBuffer, VK_SAMPLE_COUNT_1_BIT, &sampleMask);

		vkCmdSetAlphaToCoverageEnableEXT(commandBuffer, VK_FALSE);
		vkCmdSetCullModeEXT(commandBuffer, VK_CULL_MODE_NONE);
		vkCmdSetFrontFaceEXT(commandBuffer, VK_FRONT_FACE_CLOCKWISE);
		vkCmdSetDepthTestEnableEXT(commandBuffer, VK_FALSE);
		vkCmdSetDepthWriteEnableEXT(commandBuffer, VK_FALSE);
		vkCmdSetDepthBiasEnableEXT(commandBuffer, VK_FALSE);
		vkCmdSetStencilTestEnableEXT(commandBuffer, VK_FALSE);
		vkCmdSetPrimitiveTopologyEXT(commandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		vkCmdSetPrimitiveRestartEnableEXT(commandBuffer, VK_FALSE);

		VkBool32 colorBlendEnable = VK_FALSE;
		vkCmdSetColorBlendEnableEXT(commandBuffer, 0, 1, &colorBlendEnable);

		VkColorBlendEquationEXT equation;
		equation.colorBlendOp = VK_BLEND_OP_ADD;
		equation.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		equation.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		equation.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		equation.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		equation.alphaBlendOp = VK_BLEND_OP_ADD;

		vkCmdSetColorBlendEquationEXT(commandBuffer, 0, 1, &equation);

		VkColorComponentFlags colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;

		vkCmdSetColorWriteMaskEXT(commandBuffer, 0, 1, &colorWriteMask);
	}
}
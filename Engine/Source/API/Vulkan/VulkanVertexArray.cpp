#include "kepch.h"
#include "VulkanVertexArray.h"

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

	VkVertexInputBindingDescription2EXT GetBindingDescription(const BufferLayout& layout)
	{
		VkVertexInputBindingDescription2EXT bindingDescription = {};

		bindingDescription.sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
		bindingDescription.binding = 0;
		bindingDescription.stride = layout.GetStride();
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Each vertex has its own data
		bindingDescription.divisor = 1; // Instancing
		bindingDescription.pNext = nullptr; // No additional data

		//KE_CORE_INFO("Binding Description: Stride: {0}", bindingDescription.stride);

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

			//KE_CORE_INFO("Attribute: {0}, Type: {1}, Offset: {2}", element.Name, (int)element.Type, element.Offset);

			attributeDescriptions.push_back(desc);
			locationIndex++;
		}

		return attributeDescriptions;
	}

	VulkanVertexArray::VulkanVertexArray()
	{
		
	}

	VulkanVertexArray::~VulkanVertexArray()
	{
		
	}
}
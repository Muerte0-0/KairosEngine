#include "kepch.h"
#include "VulkanVertexArray.h"

#include "volk.h"

namespace Kairos
{
	static VkFormat ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
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

	VulkanVertexArray::VulkanVertexArray()
	{
		
	}

	VulkanVertexArray::~VulkanVertexArray()
	{
		
	}

	void VulkanVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		
	}

	void VulkanVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		
	}
}
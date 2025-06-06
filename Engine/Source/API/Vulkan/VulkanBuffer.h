#pragma once
#include "Engine/Renderer/Buffer.h"

#include <vulkan/vulkan.h>

namespace Kairos
{
	// Vertex Buffer //

	class VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(float* vertices, uint32_t size);
		virtual ~VulkanVertexBuffer();

		virtual void Bind() const override {};
		virtual void UnBind() const override {};

		virtual const BufferLayout& GetLayout() const override { return m_Layout; }
		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }

		VkBuffer& GetVertexBuffer() { return m_VertexBuffer; }
		VkDeviceSize& GetOffsets() { return m_Offsets; }
	private:
		BufferLayout m_Layout;

		VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;

		VkDeviceSize m_Offsets = 0;
	};

	// Index Buffer //

	class VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(uint32_t* indices, uint32_t count);
		virtual ~VulkanIndexBuffer();

		virtual void Bind() const {};
		virtual void UnBind() const {};

		virtual uint32_t GetCount() const { return m_Count; }

		VkBuffer& GetIndexBuffer() { return m_IndexBuffer; }
	private:
		uint32_t m_Count;

		VkBuffer m_IndexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_IndexBufferMemory = VK_NULL_HANDLE;
	};
}
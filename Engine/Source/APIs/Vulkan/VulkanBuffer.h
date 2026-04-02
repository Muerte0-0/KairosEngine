#pragma once
#include <vulkan/vulkan_raii.hpp>

#include "Engine/Renderer/RHI/Buffer.h"

namespace Engine
{
	// -----------------------------------------------------------------------
	// Vulkan Vertex Buffer
	// -----------------------------------------------------------------------

	class VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(const void* data, uint32_t size, BufferUsage usage = BufferUsage::Static);
		~VulkanVertexBuffer() override;
		
		const vk::raii::Buffer& GetBuffer() const { return m_Buffer; }
		const vk::raii::DeviceMemory& GetDeviceMemory() const { return m_BufferMemory; }
		
		void SetData(const void* data, uint32_t size, uint32_t offset) override;
		uint32_t GetSize() const override { return static_cast<uint32_t>(m_Size); }
		
		const BufferLayout& GetLayout() const override { return m_Layout; }
		void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
		
		BufferUsage GetUsage() const override { return m_Usage; }
		
	private:
		vk::DeviceSize m_Size;
		
		vk::raii::Buffer m_Buffer{ nullptr };
		vk::raii::DeviceMemory m_BufferMemory{ nullptr };
		void* m_MappedMemory = nullptr; // Only used with Dynamic Buffer
		
		BufferLayout m_Layout;
		BufferUsage m_Usage;
		
		void CreateBuffer_Static(const void* data, uint32_t size);
		void CreateBuffer_Dynamic(const void* data, uint32_t size);
	};
	
	// -----------------------------------------------------------------------
	// Vulkan Index Buffer
	// -----------------------------------------------------------------------
	
	class VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(const void* indices, uint32_t count, uint32_t indexSize = sizeof(uint32_t), BufferUsage usage = BufferUsage::Static);
		~VulkanIndexBuffer() override;
		
		const vk::raii::Buffer& GetBuffer() const { return m_Buffer; }
		const vk::raii::DeviceMemory& GetDeviceMemory() const { return m_BufferMemory; }
		
		uint32_t GetCount() const override { return m_Count; }
		uint32_t GetIndexSize() const override { return m_IndexSize; }
		
		BufferUsage GetUsage() const override { return m_Usage; }
		
	private:
		vk::raii::Buffer m_Buffer{ nullptr };
		vk::raii::DeviceMemory m_BufferMemory{ nullptr };
		void* m_MappedMemory = nullptr; // Only used with Dynamic Buffer
		
		uint32_t m_Count{ 0 };
		uint32_t m_IndexSize{ 0 };
		
		BufferUsage m_Usage;
		
		void CreateBuffer_Static(const void* indices, uint32_t count, uint32_t indexSize = sizeof(uint32_t));
		void CreateBuffer_Dynamic(const void* indices, uint32_t count, uint32_t indexSize = sizeof(uint32_t));
	};
}

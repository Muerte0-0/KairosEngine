#include "kepch.h"
#include "VulkanBuffer.h"

#include "Engine/Renderer/Renderer.h"

#include "VulkanRenderAPI.h"
#include "VulkanUtils.h"

namespace Engine
{
	// -----------------------------------------------------------------------
	// Vulkan Vertex Buffer
	// -----------------------------------------------------------------------

	VulkanVertexBuffer::VulkanVertexBuffer(const void* data, uint32_t size, BufferUsage usage) : m_Size(size), m_Usage(usage)
	{
		switch (m_Usage)
		{
		case BufferUsage::Static:
			CreateBuffer_Static(data, size);
			break;
		case BufferUsage::Dynamic:
			CreateBuffer_Dynamic(data, size);
			break;
		}
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		if (m_MappedMemory)
		{
			m_BufferMemory.unmapMemory();
			m_MappedMemory = nullptr;
		}
	}

	void VulkanVertexBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		if (m_Usage == BufferUsage::Dynamic)
			memcpy(static_cast<uint8_t*>(m_MappedMemory) + offset, data, size);
		else
			LOG(LogLevel::Warning, "SetData not supported for Static Buffers yet!");
	}
	
	void VulkanVertexBuffer::CreateBuffer_Static(const void* data, uint32_t size)
	{
		auto* api = dynamic_cast<VulkanRenderAPI*>(Renderer::GetAPI());
		
		vk::raii::Buffer stagingBuffer({});
		vk::raii::DeviceMemory stagingBufferMemory({});
		
		VulkanUtils::CreateBuffer(api->GetVulkanDevice()->GetDevice(), api->GetVulkanDevice()->GetPhysicalDevice(),
			m_Size, vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			stagingBuffer, stagingBufferMemory);
		
		void* dataStaging = stagingBufferMemory.mapMemory(0, m_Size);
		memcpy(dataStaging, data, m_Size);
		stagingBufferMemory.unmapMemory();
		
		VulkanUtils::CreateBuffer(api->GetVulkanDevice()->GetDevice(), api->GetVulkanDevice()->GetPhysicalDevice(),
			m_Size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal, m_Buffer, m_BufferMemory);

		VulkanUtils::CopyBuffer(api->GetVulkanDevice()->GetDevice(), api->GetVulkanCommand()->GetCommandPool(), api->GetVulkanDevice()->GetTransferQueue(),
			stagingBuffer, m_Buffer, m_Size);
	}

	void VulkanVertexBuffer::CreateBuffer_Dynamic(const void* data, uint32_t size)
	{
		auto* api = dynamic_cast<VulkanRenderAPI*>(Renderer::GetAPI());
		
		VulkanUtils::CreateBuffer(api->GetVulkanDevice()->GetDevice(), api->GetVulkanDevice()->GetPhysicalDevice(),
			m_Size, vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			m_Buffer, m_BufferMemory);
		
		m_MappedMemory = m_BufferMemory.mapMemory(0, m_Size);
		memcpy(m_MappedMemory, data, size);
	}

	// -----------------------------------------------------------------------
	// Vulkan Index Buffer
	// -----------------------------------------------------------------------

	VulkanIndexBuffer::VulkanIndexBuffer(const void* indices, uint32_t count, uint32_t indexSize, BufferUsage usage)
	: m_Count(count), m_IndexSize(indexSize), m_Usage(usage)
	{
		switch (m_Usage)
		{
		case BufferUsage::Static:
			CreateBuffer_Static(indices, count, indexSize);
			break;
		case BufferUsage::Dynamic:
			CreateBuffer_Dynamic(indices, count, indexSize);
			break;
		}
	}

	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		if (m_MappedMemory)
		{
			m_BufferMemory.unmapMemory();
			m_MappedMemory = nullptr;
		}
	}

	void VulkanIndexBuffer::CreateBuffer_Static(const void* indices, uint32_t count, uint32_t indexSize)
	{
		auto* api = dynamic_cast<VulkanRenderAPI*>(Renderer::GetAPI());
		
		vk::DeviceSize size = m_IndexSize;
		
		vk::raii::Buffer stagingBuffer({});
		vk::raii::DeviceMemory stagingBufferMemory({});
		
		VulkanUtils::CreateBuffer(api->GetVulkanDevice()->GetDevice(), api->GetVulkanDevice()->GetPhysicalDevice(),
			size, vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			stagingBuffer, stagingBufferMemory);
		
		void* data = stagingBufferMemory.mapMemory(0, size);
		memcpy(data, indices, (size_t)size);
		stagingBufferMemory.unmapMemory();

		VulkanUtils::CreateBuffer(api->GetVulkanDevice()->GetDevice(), api->GetVulkanDevice()->GetPhysicalDevice(),
			size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal, m_Buffer, m_BufferMemory);

		VulkanUtils::CopyBuffer(api->GetVulkanDevice()->GetDevice(), api->GetVulkanCommand()->GetCommandPool(), api->GetVulkanDevice()->GetTransferQueue(),
			stagingBuffer, m_Buffer, size);
	}

	void VulkanIndexBuffer::CreateBuffer_Dynamic(const void* indices, uint32_t count, uint32_t indexSize)
	{
		auto* api = dynamic_cast<VulkanRenderAPI*>(Renderer::GetAPI());
		
		vk::DeviceSize size = m_IndexSize;
		
		VulkanUtils::CreateBuffer(api->GetVulkanDevice()->GetDevice(), api->GetVulkanDevice()->GetPhysicalDevice(),
			size, vk::BufferUsageFlagBits::eIndexBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			 m_Buffer, m_BufferMemory);
		
		m_MappedMemory = m_BufferMemory.mapMemory(0, size);
		memcpy(m_MappedMemory, indices, size);
	}
}

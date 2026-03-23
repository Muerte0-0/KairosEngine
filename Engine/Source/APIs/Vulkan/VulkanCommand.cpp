#include "kepch.h"
#include "VulkanCommand.h"
#include "VulkanUtils.h"

namespace Engine
{
	VulkanCommand::VulkanCommand(VulkanDevice& device) : m_VulkanDevice(device)
	{}

	VulkanCommand::~VulkanCommand()
	{
		m_VulkanDevice.WaitIdle();
		// RAII Will Handle Cleanup 
	}

	bool VulkanCommand::CreateCommandPool()
	{
		try
		{
			vk::CommandPoolCreateInfo poolInfo;
			poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
			poolInfo.queueFamilyIndex = m_VulkanDevice.GetQueueFamilyIndices().graphicsFamily.value();

			m_CommandPool = vk::raii::CommandPool(m_VulkanDevice.GetDevice(), poolInfo);

			return true;
		}
		catch (const exception& e)
		{
			cerr << "Failed to create Command Pool: " << e.what() << "\n";
			return false;
		}
	}

	bool VulkanCommand::CreateCommandBuffers(uint32_t bufferCount)
	{
		try
		{
			m_CommandBuffers.clear();

			vk::CommandBufferAllocateInfo allocInfo;
			allocInfo.commandPool = m_CommandPool;
			allocInfo.level = vk::CommandBufferLevel::ePrimary;
			allocInfo.commandBufferCount = bufferCount;

			m_CommandBuffers = vk::raii::CommandBuffers(m_VulkanDevice.GetDevice(), allocInfo);

			return true;
		}
		catch (const exception& e)
		{
			cerr << "Failed to create Command Buffers: " << e.what() << "\n";
			return false;
		}
	}

	bool VulkanCommand::CreateSyncObjects(uint32_t imageCount, uint32_t maxFrames)
	{
		try
		{
			assert(m_PresentCompleteSemaphores.empty() && m_RenderFinishedSemaphores.empty() && m_InFlightFences.empty());

			for (size_t i = 0; i < imageCount; i++)
			{
				m_RenderFinishedSemaphores.emplace_back(m_VulkanDevice.GetDevice(), vk::SemaphoreCreateInfo());
			}

			for (size_t i = 0; i < maxFrames; i++)
			{
				m_PresentCompleteSemaphores.emplace_back(m_VulkanDevice.GetDevice(), vk::SemaphoreCreateInfo());
				m_InFlightFences.emplace_back(m_VulkanDevice.GetDevice(), vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
			}
			
			return true;
			
		}
		catch (const exception& e)
		{
			cerr << "Failed to create Command Buffers: " << e.what() << "\n";
			return false;
		}
	}

	void VulkanCommand::RecreatePresentSemaphores(uint32_t imageCount)
	{
		m_PresentCompleteSemaphores.clear();
		
		m_PresentCompleteSemaphores.reserve(imageCount);
		vk::SemaphoreCreateInfo semaphoreInfo{};
		for (size_t i = 0; i < imageCount; i++)
		{
			m_PresentCompleteSemaphores.push_back(m_VulkanDevice.GetDevice().createSemaphore(semaphoreInfo));
		}
	}
}

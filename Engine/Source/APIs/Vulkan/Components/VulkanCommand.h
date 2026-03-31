#pragma once
#include "VulkanDevice.h"

namespace Engine
{
	class VulkanCommand
	{
	public:
		/**
		  * @param device The Vulkan device.
		*/
		VulkanCommand(VulkanDevice &device);
		
		/**
		 * @brief Destructor.
		*/
		~VulkanCommand();
		
		bool CreateCommandPool();
		
		bool CreateCommandBuffers(uint32_t bufferCount);
		
		bool CreateSyncObjects(uint32_t imageCount, uint32_t maxFrames);
		
		void RecreatePresentSemaphores(uint32_t imageCount);
		
		const vk::raii::CommandPool& GetCommandPool() const { return m_CommandPool; }
		const vector<vk::raii::CommandBuffer>& GetCommandBuffers() const { return m_CommandBuffers; }
		
		const vector<vk::raii::Semaphore>& GetPresentCompleteSemaphores() const { return m_PresentCompleteSemaphores; }
		const vector<vk::raii::Semaphore>& GetRenderFinishedSemaphores() const { return m_RenderFinishedSemaphores; }
		const vector<vk::raii::Fence>& GetInFlightFences() const { return m_InFlightFences; }
		
	private:
		VulkanDevice &m_VulkanDevice;
		
		vk::raii::CommandPool m_CommandPool = nullptr;
		vector<vk::raii::CommandBuffer> m_CommandBuffers;

		vector<vk::raii::Semaphore> m_PresentCompleteSemaphores;
		vector<vk::raii::Semaphore> m_RenderFinishedSemaphores;
		vector<vk::raii::Fence> m_InFlightFences;
	};
}

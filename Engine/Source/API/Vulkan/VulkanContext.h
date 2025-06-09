#pragma once
#include "Engine/Renderer/GraphicsContext.h"

#include <volk.h>
#include <vma/vk_mem_alloc.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Components
#include "Components/Instance.h"
#include "Components/Swapchain.h"
#include "Components/Frame.h"

namespace Kairos
{
	struct VkContext 
	{
		VkInstance Instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT Messenger = VK_NULL_HANDLE;

		VkSurfaceKHR Surface = VK_NULL_HANDLE;
		VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
		VkDevice LogicalDevice = VK_NULL_HANDLE;

		VmaAllocator Allocator = VK_NULL_HANDLE;
		VkAllocationCallbacks* AllocationCallbacks = VK_NULL_HANDLE;

		VkQueue GraphicsQueue = VK_NULL_HANDLE;
		VkQueue PresentQueue = VK_NULL_HANDLE;
		VkQueue ComputeQueue = VK_NULL_HANDLE;

		Swapchain Swapchain;
		std::vector<Frame> Frames;

		VkCommandPool CommandPool = VK_NULL_HANDLE;

		VkSampler Sampler = VK_NULL_HANDLE;

		VkDescriptorSetLayout DescriptorSetLayout = VK_NULL_HANDLE;
		VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;

		VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;

		VkSemaphore ImageAquiredSemaphore = VK_NULL_HANDLE;
		VkSemaphore RenderFinishedSemaphore = VK_NULL_HANDLE;
		VkFence RenderFinishedFence = VK_NULL_HANDLE;
	};

	static uint16_t CurrentFrameIndex = 0;

	class VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SetVSync(bool enabled) override;
		virtual void SwapBuffers() override;
		virtual void Cleanup() override;

		std::deque<std::function<void(VkInstance)>>& GetInstanceDeletionQueue() { return m_InstanceDeletionQueue; }
		std::deque<std::function<void(VkDevice)>>& GetDeviceDeletionQueue() { return m_DeviceDeletionQueue; }

		VkContext& GetVkContext() { return m_Context; }

		GLFWwindow* GetWindowHandle() {return m_WindowHandle;}
	private:
		GLFWwindow* m_WindowHandle;

		VkContext m_Context;

		void Init_Allocator();

		std::deque<std::function<void(VkInstance)>> m_InstanceDeletionQueue;
		std::deque<std::function<void(VkDevice)>> m_DeviceDeletionQueue;
	};
}
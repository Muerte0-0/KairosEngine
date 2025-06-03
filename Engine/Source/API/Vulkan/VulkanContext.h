#pragma once
#include "Engine/Renderer/GraphicsContext.h"

#include <vulkan/vulkan.hpp>
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
		VkInstance Instance;
		VkDebugUtilsMessengerEXT Messenger;

		VkSurfaceKHR Surface = VK_NULL_HANDLE;
		VkPhysicalDevice PhysicalDevice;
		VkDevice LogicalDevice;

		VmaAllocator Allocator = VK_NULL_HANDLE;
		VkAllocationCallbacks* AllocationCallbacks;

		VkQueue GraphicsQueue;
		VkQueue PresentQueue;
		VkQueue ComputeQueue;

		Swapchain Swapchain;
		std::vector<Frame> Frames;

		VkCommandPool CommandPool = VK_NULL_HANDLE;

		VkSemaphore ImageAquiredSemaphore;
		VkSemaphore RenderFinishedSemaphore;
		VkFence RenderFinishedFence;
	};

#define VK_CHECK(result) \
    if (result != VK_SUCCESS) { \
        KE_CORE_ERROR("Vulkan error: {}", (char*)result); \
        abort(); \
    }

#define MAX_FRAMES_IN_FLIGHT 2

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
#pragma once
#include "Engine/Renderer/GraphicsContext.h"

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Components
#include "Components/Swapchain.h"

namespace Kairos
{
	struct QueueFamilyIndices
	{
		uint32_t graphics_family;
		uint32_t present_family;
		uint32_t compute_family;
		bool has_compute;
	};

	struct VkContext 
	{
		VkInstance instance;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkPhysicalDevice physicalDevice;
		VkDevice device;

		VmaAllocator allocator = VK_NULL_HANDLE;
		VkAllocationCallbacks* callbacks;

		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkQueue computeQueue;

		Swapchain m_Swapchain;

		VkCommandPool commandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> commandBuffers;

		std::vector<VkSemaphore> imageAvailableSemaphores; // Signaled when swapchain image is ready
		std::vector<VkSemaphore> renderFinishedSemaphores; // Signaled when rendering completes
		std::vector<VkFence> inFlightFences;               // CPU-GPU synchronization
		std::vector<VkFence> imagesInFlight;               // Track in-use swapchain images
		size_t currentFrame = 0;
	};

#define VK_CHECK(result) \
    if (result != VK_SUCCESS) { \
        KE_CORE_ERROR("Vulkan error: {}", (char*)result); \
        abort(); \
    }

#define MAX_FRAMES_IN_FLIGHT 2

	class VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SetVSync(bool enabled) override;
		virtual void SwapBuffers() override;
		virtual void Cleanup() override;

		QueueFamilyIndices FindQueueFamilies();

		std::deque<std::function<void(VkInstance)>>& GetInstanceDeletionQueue() { return m_InstanceDeletionQueue; }
		std::deque<std::function<void(VkDevice)>>& GetDeviceDeletionQueue() { return m_DeviceDeletionQueue; }

		VkContext& GetVkContext() { return m_Context; }

		GLFWwindow* GetWindowHandle() {return m_WindowHandle;}
	private:
		GLFWwindow* m_WindowHandle;

		VkContext m_Context;

		void Init_Instance();

		void Init_Device();
		void SelectPhysicalDevice();
		void CreateLogicalDevice();

		void Init_Allocator();

		std::deque<std::function<void(VkInstance)>> m_InstanceDeletionQueue;
		std::deque<std::function<void(VkDevice)>> m_DeviceDeletionQueue;
	};
}
#pragma once
#include "Engine/Renderer/GraphicsContext.h"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Kairos
{
	typedef struct {
		uint32_t graphics_family;
		uint32_t present_family;
		uint32_t compute_family;  // Optional async compute
		bool has_compute;
	} QueueFamilyIndices;

	typedef struct {
		VkInstance instance;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice;
		VkDevice device;

		VmaAllocator allocator;
		VkAllocationCallbacks* callbacks;

		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkQueue computeQueue;
	} VulkanVariables;

	static struct {
		size_t totalAllocated = 0;
		std::mutex mutex;
	} allocStats;

#define VK_CHECK(result) \
    if (result != VK_SUCCESS) { \
        KE_CORE_ERROR("Vulkan error: {}", (char*)result); \
        abort(); \
    }

	class VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SetVSync(bool enabled) override;
		virtual void SwapBuffers() override;
		virtual void Cleanup() override;

		std::deque<std::function<void(VkDevice)>>& GetDeviceDeletionQueue() { return m_DeviceDeletionQueue; }

		VulkanVariables* GetVulkanVariables() { return &m_VulkanVariables; }
	private:
		GLFWwindow* m_WindowHandle;

		VulkanVariables m_VulkanVariables;

		void Init_Instance();

		void Init_Device();
		void SelectPhysicalDevice();
		void CreateLogicalDevice();

		QueueFamilyIndices FindQueueFamilies();

		void Init_Allocator();

		std::deque<std::function<void(VkInstance)>> m_InstanceDeletionQueue;
		std::deque<std::function<void(VkDevice)>> m_DeviceDeletionQueue;
	};
}
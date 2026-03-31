#pragma once
#include <vulkan/vulkan_raii.hpp>

namespace Engine
{
	/**
	* @brief Structure for Vulkan Queue Family Indices.
	*/
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		std::optional<uint32_t> computeFamily;
		std::optional<uint32_t> transferFamily; // optional dedicated transfer queue family

		[[nodiscard]] bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value(); }
	};
	
	class VulkanDevice
	{
	public:
		/**
		 * @param instance The Vulkan instance.
		 * @param surface The Vulkan surface.
		 * @param requiredExtensions The required device extensions.
		 * @param optionalExtensions The optional device extensions.
		*/
		VulkanDevice(vk::raii::Instance &instance, vk::raii::SurfaceKHR &surface,
			const std::vector<const char *> &requiredExtensions,
			const std::vector<const char *> &optionalExtensions = {});
		
		~VulkanDevice();

		/**
		 * @brief Pick a suitable physical device.
		 * @return True if a suitable device was found, false otherwise.
		*/
		bool PickPhysicalDevice();

		/**
		 * @brief Create a logical device.
		 * @param enableValidationLayers Whether to enable validation layers.
		 * @param validationLayers The validation layers to enable.
		 * @return True if the logical device was created successfully, false otherwise.
		*/
		bool CreateLogicalDevice(bool enableValidationLayers, const std::vector<const char *> &validationLayers);

		void WaitIdle() const { m_Device.waitIdle(); }
		
		/**
		 * @brief Get the Instance.
		 * @return The Instance.
		*/
		vk::raii::Instance &GetInstance() const { return m_Instance; }
		
		
		/**
		 * @brief Get the Surface.
		 * @return The Surface KHR.
		*/
		vk::raii::SurfaceKHR &GetSurface() const { return m_Surface; }
		
		/**
		 * @brief Get the physical device.
		 * @return The physical device.
		*/
		vk::raii::PhysicalDevice &GetPhysicalDevice() { return m_PhysicalDevice; }

		/**
		 * @brief Get the logical device.
		 * @return The logical device.
		*/
		vk::raii::Device &GetDevice() { return m_Device; }

		/**
		 * @brief Get the graphics queue.
		 * @return The graphics queue.
		*/
		vk::raii::Queue &GetGraphicsQueue() { return m_GraphicsQueue; }

		/**
		 * @brief Get the present queue.
		 * @return The present queue.
		*/
		vk::raii::Queue &GetPresentQueue() { return m_PresentQueue; }

		/**
		 * @brief Get the compute queue.
		 * @return The compute queue.
		*/
		vk::raii::Queue &GetComputeQueue() { return m_ComputeQueue; }

		/**
		 * @brief Get the queue family indices.
		 * @return The queue family indices.
		*/
		QueueFamilyIndices GetQueueFamilyIndices() const { return m_QueueFamilyIndices; }

		/**
		 * @brief Find queue families for a physical device.
		 * @param device The physical device.
		 * @return The queue family indices.
		*/
		QueueFamilyIndices FindQueueFamilies(const vk::raii::PhysicalDevice &device) const;

  private:
		vk::raii::Instance &m_Instance;
		vk::raii::SurfaceKHR &m_Surface;
		
		// Vulkan device
		vk::raii::PhysicalDevice m_PhysicalDevice = nullptr;
		vk::raii::Device m_Device = nullptr;
		
		// Vulkan queues
		vk::raii::Queue m_GraphicsQueue = nullptr;
		vk::raii::Queue m_PresentQueue = nullptr;
		vk::raii::Queue m_ComputeQueue = nullptr;
		
		// Queue family indices
		QueueFamilyIndices m_QueueFamilyIndices;
		
		// Device extensions
		std::vector<const char *> m_RequiredExtensions;
		std::vector<const char *> m_OptionalExtensions;
		std::vector<const char *> m_DeviceExtensions;
		
		// Private methods
		bool IsDeviceSuitable(const vk::raii::PhysicalDevice &device);
		bool CheckDeviceExtensionSupport(const vk::raii::PhysicalDevice &device);
};
}

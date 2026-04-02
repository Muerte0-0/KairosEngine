#include "kepch.h"
#include "VulkanDevice.h"
#include "APIs/Vulkan/VulkanUtils.h"

namespace Engine
{
	VulkanDevice::VulkanDevice(vk::raii::Instance& instance, vk::raii::SurfaceKHR& surface, 
		const std::vector<const char*>& requiredExtensions,
		const std::vector<const char*>& optionalExtensions) : m_Instance(instance), m_Surface(surface),
		m_RequiredExtensions(requiredExtensions), m_OptionalExtensions(optionalExtensions)
	{
		// Initialize m_DeviceExtensions with required extensions
		m_DeviceExtensions = requiredExtensions;

		// Add optional extensions
		m_DeviceExtensions.insert(m_DeviceExtensions.end(), optionalExtensions.begin(), optionalExtensions.end());
	}

	VulkanDevice::~VulkanDevice()
	{
		// RAII Will Handle Destruction
	}

	bool VulkanDevice::PickPhysicalDevice()
	{
		try
		{
			// Get available physical devices
			std::vector<vk::raii::PhysicalDevice> devices = m_Instance.enumeratePhysicalDevices();

			if (devices.empty()) 
			{
				LOG(LogLevel::Error, "Failed to find GPUs with Vulkan support");
				return false;
			}
			
			// Find a suitable device using modern C++ ranges
			const auto devIter= std::ranges::find_if(devices, [&](auto& device) 
			{
				// Print device properties for debugging
				vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
				LOG(LogLevel::Info, "Checking Device: {}", deviceProperties.deviceName.data());

				// Check if device supports Vulkan 1.3
				bool supportsVulkan1_3 = deviceProperties.apiVersion >= vk::ApiVersion13;
				if (!supportsVulkan1_3) LOG(LogLevel::Warning, "  - Does not support Vulkan 1.3");

				// Check queue families
				QueueFamilyIndices indices = FindQueueFamilies(device);
				bool supportsGraphics = indices.isComplete();
				if (!supportsGraphics) LOG(LogLevel::Warning, "  - Missing required queue families");

				// Check device extensions
				bool supportsAllRequiredExtensions = CheckDeviceExtensionSupport(device);
				if (!supportsAllRequiredExtensions) LOG(LogLevel::Warning, "  - Missing required extensions");
			  	
				// Check swap chain support
				bool swapChainAdequate = false;
	   			if (supportsAllRequiredExtensions)
	   			{
	   				SwapChainSupportDetails swapChainSupport = VulkanUtils::QuerySwapChainSupport(device, m_Surface);
	   				swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	   				if (!swapChainAdequate) LOG(LogLevel::Warning, "  - Inadequate swap chain support");
	   			}

				// Check for required features
				auto features = device.template getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features>();
				bool supportsRequiredFeatures = (features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering == VK_TRUE);
				if (!supportsRequiredFeatures) LOG(LogLevel::Warning, "  - Does not support required features (dynamicRendering)");

				return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && swapChainAdequate && supportsRequiredFeatures;
			});
			
			if (devIter != devices.end())
			{
				m_PhysicalDevice = *devIter;
				vk::PhysicalDeviceProperties deviceProperties = m_PhysicalDevice.getProperties();
				LOG(LogLevel::Info, "Selected device: {}", deviceProperties.deviceName.data());

				// Store queue family indices for the selected device
				m_QueueFamilyIndices = FindQueueFamilies(m_PhysicalDevice);
				return true;
			} else
			{
				LOG(LogLevel::Error, "Failed to find a suitable GPU. Make sure your GPU supports Vulkan and has the required extensions.");
				return false;
			}
			
		} catch (const exception& e)
		{
			LOG(LogLevel::Error, "Failed to pick physical device: {}", e.what());
			return false;
		}
	}

	bool VulkanDevice::CreateLogicalDevice(bool enableValidationLayers, const std::vector<const char*>& validationLayers)
	{
		try
		{
			// Create queue create infos for each unique queue family
			std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
			std::set<uint32_t> uniqueQueueFamilies = {
				m_QueueFamilyIndices.graphicsFamily.value(),
				m_QueueFamilyIndices.presentFamily.value(),
				m_QueueFamilyIndices.computeFamily.value(),
				m_QueueFamilyIndices.transferFamily.value()
			  };
			
			float queuePriority = 1.0f;
			
			for (uint32_t queueFamily : uniqueQueueFamilies)
			{
				vk::DeviceQueueCreateInfo queueCreateInfo;
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;
				
				queueCreateInfos.push_back(queueCreateInfo);
			}
			
			// Enable required features using StructureChain
			vk::StructureChain<
				vk::PhysicalDeviceFeatures2,
				vk::PhysicalDeviceVulkan11Features,
				vk::PhysicalDeviceVulkan13Features,
				vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain;

			featureChain.get<vk::PhysicalDeviceFeatures2>().features.setDepthClamp(VK_TRUE);
			featureChain.get<vk::PhysicalDeviceFeatures2>().features.setSamplerAnisotropy(VK_TRUE);
			featureChain.get<vk::PhysicalDeviceFeatures2>().features.setSampleRateShading(VK_TRUE);

			featureChain.get<vk::PhysicalDeviceVulkan11Features>().setShaderDrawParameters(VK_TRUE);

			featureChain.get<vk::PhysicalDeviceVulkan13Features>().setDynamicRendering(VK_TRUE);
			featureChain.get<vk::PhysicalDeviceVulkan13Features>().setSynchronization2(VK_TRUE);

			featureChain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().setExtendedDynamicState(VK_TRUE);
			
			auto& features = featureChain.get<vk::PhysicalDeviceFeatures2>();
			
			// Create device. Device layers are deprecated and ignored, so we
			// configure only extensions and features; validation is enabled via
			// instance layers.
			vk::DeviceCreateInfo createInfo;
			createInfo.pNext = &features;
			createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			createInfo.pQueueCreateInfos = queueCreateInfos.data();
			createInfo.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size());
			createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();
			createInfo.pEnabledFeatures = nullptr;	// Using pNext for features
			
			// Create the logical device
			m_Device = vk::raii::Device(m_PhysicalDevice, createInfo);
			
			VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_Device);
			
			// Get queue handles
			m_GraphicsQueue = vk::raii::Queue(m_Device, m_QueueFamilyIndices.graphicsFamily.value(), 0);
			m_PresentQueue = vk::raii::Queue(m_Device, m_QueueFamilyIndices.presentFamily.value(), 0);
			m_ComputeQueue = vk::raii::Queue(m_Device, m_QueueFamilyIndices.computeFamily.value(), 0);
			m_TransferQueue = vk::raii::Queue(m_Device, m_QueueFamilyIndices.transferFamily.value(), 0);
			
			return true;
			
		} catch(const std::exception& e)
		{
			LOG(LogLevel::Critical, "Failed to Create Logical Device: {}", e.what());
			return false;
		}
	}

	QueueFamilyIndices VulkanDevice::FindQueueFamilies(const vk::raii::PhysicalDevice& device) const
	{
		QueueFamilyIndices indices;

		// Get queue family properties
		std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

		// Find queue families that support graphics, compute, and present
		for (uint32_t i = 0; i < queueFamilies.size(); i++) 
		{
			// Check for graphics support
			if ((queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) && !indices.graphicsFamily.has_value())
				indices.graphicsFamily = i;

			// Check for compute support
			if ((queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) && !indices.computeFamily.has_value())
				indices.computeFamily = i;

			// Check for present support
			if (!indices.presentFamily.has_value() && device.getSurfaceSupportKHR(i, *m_Surface))
				indices.presentFamily = i;
			
			// Prefer a dedicated transfer queue (transfer bit set, but NOT graphics) if available
			if ((queueFamilies[i].queueFlags & vk::QueueFlagBits::eTransfer) && !(queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics))
				if (!indices.transferFamily.has_value())
					indices.transferFamily = i;

			// If all required queue families are found, we can still continue to try to find a dedicated transfer queue
			if (indices.isComplete() && indices.transferFamily.has_value())
				// Found everything including dedicated transfer
				break;
		}
		
		// Fallback: if no dedicated transfer queue, reuse graphics queue for transfer
		if (!indices.transferFamily.has_value() && indices.graphicsFamily.has_value())
			indices.transferFamily = indices.graphicsFamily;

		return indices;
	}

	bool VulkanDevice::IsDeviceSuitable(const vk::raii::PhysicalDevice& device)
	{
		// Check queue families
		QueueFamilyIndices indices = FindQueueFamilies(device);

		// Check device extensions
		bool extensionsSupported = CheckDeviceExtensionSupport(device);

		// Check swap chain support
		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = VulkanUtils::QuerySwapChainSupport(m_PhysicalDevice, m_Surface);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		// Check for required features
		auto features = device.template getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features>();
		bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering;

		return indices.isComplete() && extensionsSupported && swapChainAdequate && supportsRequiredFeatures;
	}

	bool VulkanDevice::CheckDeviceExtensionSupport(const vk::raii::PhysicalDevice& device)
	{
		// Get available extensions
		std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();

		// Only check for required extensions, not optional ones
		std::set<std::string> requiredExtensionsSet(m_RequiredExtensions.begin(), m_RequiredExtensions.end());

		for (const auto& extension : availableExtensions)
			requiredExtensionsSet.erase(extension.extensionName);

		// Print missing required extensions
		if (!requiredExtensionsSet.empty())
		{
			LOG(LogLevel::Warning, "Missing required extensions:");
			
			for (const auto& extension : requiredExtensionsSet)
				LOG(LogLevel::Warning, "{}", extension.c_str());
			
			return false;
		}

		return true;
	}
}

#include "kepch.h"
#include "Device.h"

#include "API/Vulkan/VulkanUtils.h"

#include "volk.h"

namespace Kairos
{
	bool Supports(const VkPhysicalDevice physicalDevice, const char** requestedExtensions, const uint32_t requestedExtensionCount)
	{
		uint32_t extentionsCount;
		std::vector<VkExtensionProperties> extensions;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extentionsCount, nullptr);
		extensions.resize(extentionsCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extentionsCount, extensions.data());
		

		for (uint32_t i = 0; i < requestedExtensionCount; ++i)
		{
			bool supported = false;

			for (VkExtensionProperties& extension : extensions)
			{
				std::string name = extension.extensionName;

				if (!name.compare(requestedExtensions[i]))
				{
					supported = true;
					break;
				}
			}

			if (!supported)
				return false;
		}

		return true;
	}

	bool IsSuitable(const VkPhysicalDevice physicalDevice)
	{
		KE_CORE_INFO("Choosing Physical GPU...");
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);

		KE_CORE_INFO("Selected Device: {0}", (std::string)properties.deviceName);
		switch (properties.deviceType)
		{
		case VK_PHYSICAL_DEVICE_TYPE_CPU: KE_CORE_INFO("Selected Device Type -> CPU"); break;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: KE_CORE_INFO("Selected Device Type -> Discrete GPU"); break;
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: KE_CORE_INFO("Selected Device Type -> Integrated GPU"); break;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: KE_CORE_INFO("Selected Device Type -> Virtual GPU"); break;

		default: KE_CORE_INFO("Other");
		}

		const char* requestedExtension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

		if (Supports(physicalDevice, &requestedExtension, 1))
			KE_CORE_INFO("Device Supports Requested Extensions!");
		else
		{
			KE_CORE_ERROR("Device Does Not Support Requested Extensions!");
			return false;
		}

		return true;
	}

	VkPhysicalDevice ChoosePhysicalDevice(VkInstance& instance)
	{
		uint32_t availableDevicesCount;
		std::vector<VkPhysicalDevice> availableDevices;
		vkEnumeratePhysicalDevices(instance, &availableDevicesCount, nullptr);
		availableDevices.resize(availableDevicesCount);
		vkEnumeratePhysicalDevices(instance, &availableDevicesCount, availableDevices.data());

		for (VkPhysicalDevice device : availableDevices)
			if (IsSuitable(device))
				return device;

		KE_CORE_ERROR("Suitable GPU not Found!");
		return nullptr;
	}

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
	{
		QueueFamilyIndices indices = { 0 };

		indices.GraphicsIndex = UINT32_MAX;
		indices.PresentIndex = UINT32_MAX;
		indices.ComputeIndex = UINT32_MAX;

		uint32_t queueFamiliesCount = 0;
		std::vector<VkQueueFamilyProperties> queueFamilies;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, NULL);
		queueFamilies.resize(queueFamiliesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, queueFamilies.data());

		for (uint32_t i = 0; i < queueFamiliesCount; i++)
		{
			// Graphics queue
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.GraphicsIndex = i;
			}

			// Present queue
			VkBool32 presentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
			if (presentSupport)
			{
				indices.PresentIndex = i;
			}

			// Dedicated compute queue (no graphics capability)
			if ((queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && !(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				indices.ComputeIndex = i;
				indices.HasCompute = true;
			}
		}

		return indices;
	}

	uint32_t FindQueueFamilyIndex(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueueFlags queueType)
	{
		uint32_t queueFamiliesCount;
		std::vector<VkQueueFamilyProperties> queueFamilies;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, nullptr);
		queueFamilies.resize(queueFamiliesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, queueFamilies.data());

		for (uint32_t i = 0; i < queueFamilies.size(); ++i)
		{
			VkQueueFamilyProperties queueFamily = queueFamilies[i];

			VkBool32 canPresent = VK_FALSE;
			if (surface)
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &canPresent);

			bool supported = false;
			if (queueFamily.queueFlags & queueType)
				supported = true;

			if (supported && canPresent)
				return i;
		}

		return UINT32_MAX;
	}

	VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::deque<std::function<void(VkDevice)>>& deviceDeletionQueue)
	{
		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);

		std::vector<float> queuePriorities = { 1.0f };

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

		// Graphics queue (may also support present)
		queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.flags = VkDeviceQueueCreateFlags(),
				.queueFamilyIndex = indices.GraphicsIndex,
				.queueCount = 1,
				.pQueuePriorities = queuePriorities.data(),
			});


		// Separate present queue if needed
		if (indices.PresentIndex != indices.GraphicsIndex)
		{
			queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.flags = VkDeviceQueueCreateFlags(),
				.queueFamilyIndex = indices.PresentIndex,
				.queueCount = 1,
				.pQueuePriorities = queuePriorities.data(),
				});
		}

		// Separate compute queue if available
		if (indices.HasCompute && indices.ComputeIndex != indices.GraphicsIndex && indices.ComputeIndex != indices.PresentIndex)
		{
			queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.flags = VkDeviceQueueCreateFlags(),
				.queueFamilyIndex = indices.ComputeIndex,
				.queueCount = 1,
				.pQueuePriorities = queuePriorities.data(),
				});
		}

		VkPhysicalDeviceFeatures deviceFeatures =
		{
			.multiViewport = VK_TRUE,
		};

		VkPhysicalDeviceFeatures2 deviceFeatures2 = 
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.features = deviceFeatures,
		};

		VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures =
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
			.shaderObject = VK_TRUE,
		};
		

		VkPhysicalDeviceShaderDrawParametersFeatures shaderDrawParamsFeatures = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES,
			.shaderDrawParameters = VK_TRUE
		};

		VkPhysicalDeviceVulkan13Features vulkan13Features = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.synchronization2 = VK_TRUE,
			.dynamicRendering = VK_TRUE,
		};

		VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT graphicsPipelineLibraryFeatures = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT,
			.graphicsPipelineLibrary = VK_TRUE,
		};

		deviceFeatures2.pNext = &shaderObjectFeatures;
		shaderObjectFeatures.pNext = &shaderDrawParamsFeatures;
		shaderDrawParamsFeatures.pNext = &graphicsPipelineLibraryFeatures;
		graphicsPipelineLibraryFeatures.pNext = &vulkan13Features;


		std::vector<const char*> enabledLayers;
#ifdef KE_DEBUG
		enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
		enabledLayers.push_back("VK_LAYER_KHRONOS_synchronization");
#endif // KE_DEBUG

		std::vector<const char*> enabledExtensions
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
			VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
			VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
			VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
			VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME,
			VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME,
			VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
		};

		VkDeviceCreateInfo deviceCreateInfo;

		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.flags = VkDeviceCreateFlags();
		deviceCreateInfo.pEnabledFeatures = nullptr;
		deviceCreateInfo.pNext = &deviceFeatures2;

		deviceCreateInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

		deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

		deviceCreateInfo.enabledLayerCount = (uint32_t)enabledLayers.size();
		deviceCreateInfo.ppEnabledLayerNames = enabledLayers.data();

		VkDevice logicalDevice;

		VK_CHECK(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice));

		deviceDeletionQueue.push_back([](VkDevice device)
			{
				vkDestroyDevice(device, nullptr);
				KE_CORE_INFO("Deleted Logical Device!");
			});

		return logicalDevice;
	}
}
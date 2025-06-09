#pragma once

#include <vulkan/vulkan.h>

namespace Kairos
{
	VkDescriptorPool CreateDescriptorPool(VkDevice logicalDevice, std::deque<std::function<void(VkDevice)>>& deviceDeletionQueue);

	class DescriptorSetLayoutBuilder
	{
	public:
		DescriptorSetLayoutBuilder(VkDevice& logicalDevice);

		VkDescriptorSetLayout Build(std::deque<std::function<void(VkDevice)>>& deletionQueue);

		void AddEntry(VkShaderStageFlags stage, VkDescriptorType type);
	private:
		VkDevice& m_LogicalDevice;
		std::vector<VkDescriptorSetLayoutBinding> m_LayoutBindings;

		void Reset();
	};
}
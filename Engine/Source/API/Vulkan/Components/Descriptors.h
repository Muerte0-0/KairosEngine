#pragma once

#include <vulkan/vulkan.h>

namespace Kairos
{
	class DescriptorSetLayoutBuilder
	{
	public:
		DescriptorSetLayoutBuilder(VkDevice& logicalDevice);

		VkDescriptorSetLayout Build(std::deque<std::function<void(VkInstance)>>& deletionQueue);

		void AddEntry(VkShaderStageFlags stage, VkDescriptorType type);
	private:
		VkDevice& m_LogicalDevice;

		std::vector<VkDescriptorSetLayoutBinding> m_LayoutBindings;

		void Reset();
	};
}
#include "kepch.h"

#include "Descriptors.h"

namespace Kairos
{
	DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder(VkDevice& logicalDevice) : m_LogicalDevice(logicalDevice)
	{
		
	}

	VkDescriptorSetLayout DescriptorSetLayoutBuilder::Build(std::deque<std::function<void(VkInstance)>>& deletionQueue)
	{
		return VkDescriptorSetLayout();
	}

	void DescriptorSetLayoutBuilder::AddEntry(VkShaderStageFlags stage, VkDescriptorType type)
	{
		//vkDescriprotSetLayoutBinding
	}

	void DescriptorSetLayoutBuilder::Reset()
	{
		m_LayoutBindings.clear();
	}
}
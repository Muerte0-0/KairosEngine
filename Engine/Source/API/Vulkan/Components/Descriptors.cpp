#include "kepch.h"

#include "Descriptors.h"

#include "API/Vulkan/VulkanUtils.h"

#include "volk.h"

namespace Kairos
{
	DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder(VkDevice& logicalDevice) : m_LogicalDevice(logicalDevice)
	{
		
	}

	VkDescriptorSetLayout DescriptorSetLayoutBuilder::Build(std::deque<std::function<void(VkDevice)>>& deletionQueue)
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
		layoutInfo.bindingCount = static_cast<uint32_t>(m_LayoutBindings.size());
		layoutInfo.pBindings = m_LayoutBindings.data();

		VkDescriptorSetLayout layout;
		VK_CHECK(vkCreateDescriptorSetLayout(m_LogicalDevice, &layoutInfo, nullptr, &layout));

		Reset();

		deletionQueue.push_back([layout](VkDevice device)
		{
			vkDestroyDescriptorSetLayout(device, layout, nullptr);
			KE_CORE_INFO("Destroyed Descriptor Set Layout");
		});

		return layout;
	}

	void DescriptorSetLayoutBuilder::AddEntry(VkShaderStageFlags stage, VkDescriptorType type)
	{
		VkDescriptorSetLayoutBinding entry = {};
		entry.binding = m_LayoutBindings.size();
		entry.descriptorCount = 1;
		entry.descriptorType = type;
		entry.stageFlags = stage;

		m_LayoutBindings.push_back(entry);
	}

	void DescriptorSetLayoutBuilder::Reset()
	{
		m_LayoutBindings.clear();
	}
}
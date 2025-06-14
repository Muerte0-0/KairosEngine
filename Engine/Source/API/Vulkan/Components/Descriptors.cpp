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
		layoutInfo.bindingCount = (uint32_t)m_LayoutBindings.size();
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
		VkDescriptorSetLayoutBinding entry = { VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT };
		entry.binding = m_LayoutBindings.size();
		entry.descriptorType = type;
		entry.descriptorCount = 1;
		entry.stageFlags = stage;

		m_LayoutBindings.push_back(entry);
	}

	void DescriptorSetLayoutBuilder::Reset()
	{
		m_LayoutBindings.clear();
	}

	VkDescriptorPool CreateDescriptorPool(VkDevice logicalDevice, std::deque<std::function<void(VkDevice)>>& deviceDeletionQueue)
	{
		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 1000 * ((int)(sizeof(poolSizes) / sizeof(*(poolSizes))));
		poolInfo.poolSizeCount = (uint32_t)((int)(sizeof(poolSizes) / sizeof(*(poolSizes))));
		poolInfo.pPoolSizes = poolSizes;

		VkDescriptorPool pool;

		VK_CHECK(vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &pool));

		deviceDeletionQueue.push_back([pool](VkDevice device)
		{
			vkDestroyDescriptorPool(device, pool, nullptr);
		});

		return pool;
	}
}
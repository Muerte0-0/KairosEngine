#include "kepch.h"
#include "GraphicsPipeline.h"

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
#include "APIs/Vulkan/VulkanGraphicsPipeline.h"
#endif

namespace Engine
{
	GraphicsPipeline::GraphicsPipeline(GraphicsPipelineCreateInfo createInfo)
		: m_CreateInfo(std::move(createInfo))
	{}

	Scope<GraphicsPipeline> GraphicsPipeline::Create(GraphicsPipelineCreateInfo createInfo)
	{
		KE_CORE_ASSERT(createInfo.Shader, "GraphicsPipeline::Create — createInfo.Shader is null.");

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
		auto pipeline = CreateScope<VulkanGraphicsPipeline>(std::move(createInfo));
		pipeline->Init();
		return pipeline;
#else
		KE_CORE_ASSERT(false, "GraphicsPipeline::Create — unsupported platform!");
		return nullptr;
#endif
	}
}

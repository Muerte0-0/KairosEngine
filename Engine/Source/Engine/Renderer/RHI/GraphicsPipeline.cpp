#include "kepch.h"
#include "GraphicsPipeline.h"

namespace Engine
{
	GraphicsPipeline::GraphicsPipeline(GraphicsPipelineCreateInfo createInfo)
		: m_CreateInfo(std::move(createInfo))
	{
	}
}

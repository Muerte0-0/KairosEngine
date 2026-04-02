#include "kepch.h"
#include "GraphicsPipeline.h"

#include "Engine/Renderer/Renderer.h"

#include "APIs/Vulkan/VulkanGraphicsPipeline.h"

namespace Engine
{
	GraphicsPipeline::GraphicsPipeline(GraphicsPipelineCreateInfo createInfo)
		: m_CreateInfo(std::move(createInfo))
	{}

	Scope<GraphicsPipeline> GraphicsPipeline::Create(GraphicsPipelineCreateInfo createInfo)
	{
		ASSERT(createInfo.Shader, "GraphicsPipeline::Create — createInfo.Shader is null.");

		Scope<GraphicsPipeline> result = nullptr;
		
		switch (Renderer::GetAPI()->GetType())
		{
		case API::Vulkan:
			result = CreateScope<VulkanGraphicsPipeline>(std::move(createInfo));
			break;
#ifdef PLATFORM_WINDOWS
		case API::DX11:
			ASSERT(false, "API[Direct X 11]: Not Implemented");
			break;
		case API::DX12:
			ASSERT(false, "API[Direct X 12]: Not Implemented");
			break;
#endif
		}
		
		return result;
	}
}

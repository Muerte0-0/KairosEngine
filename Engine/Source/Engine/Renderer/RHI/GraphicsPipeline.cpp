#include "kepch.h"
#include "GraphicsPipeline.h"

#include "Engine/Renderer/Renderer.h"

#include "APIs/Vulkan/VulkanGraphicsPipeline.h"

namespace Engine
{
	Scope<GraphicsPipeline> GraphicsPipeline::Create(GraphicsPipelineCreateInfo createInfo)
	{
		ASSERT(createInfo.Shader, "GraphicsPipeline::Create — createInfo.Shader is null.");
		
		switch (Renderer::GetAPI()->GetType())
		{
		case API::Vulkan:
			return CreateScope<VulkanGraphicsPipeline>(std::move(createInfo));
#ifdef PLATFORM_WINDOWS
		case API::DX11:
			ASSERT(false, "API[Direct X 11]: Not Implemented");
			break;
		case API::DX12:
			ASSERT(false, "API[Direct X 12]: Not Implemented");
			break;
#endif
		}
		
		return nullptr;
	}
}

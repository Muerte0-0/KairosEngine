#include "kepch.h"
#include "VulkanRenderAPI.h"

#include "VulkanContext.h"

#include "volk.h"

namespace Kairos
{

	void VulkanRenderAPI::Init()
	{
		
	}

	void VulkanRenderAPI::SetViewport(GraphicsContext* ctx, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		VulkanContext* vctx = (VulkanContext*)ctx;

		m_Swapchain.RecreateSwapchain(vctx, width, height);
	}

	void VulkanRenderAPI::SetClearColor(const glm::vec4& color)
	{
		
	}

	void VulkanRenderAPI::Clear()
	{
		
	}

	void VulkanRenderAPI::DrawIndexed(const Ref<VertexArray>& vertexArray)
	{

	}
}
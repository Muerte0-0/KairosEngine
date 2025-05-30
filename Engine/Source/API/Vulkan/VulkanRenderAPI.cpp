#include "kepch.h"
#include "VulkanRenderAPI.h"

#include "Engine/Core/Application.h"
#include "VulkanContext.h"
#include "Components/Command.h"

#include "volk.h"


namespace Kairos
{

	void VulkanRenderAPI::Init()
	{
		
	}

	void VulkanRenderAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		VulkanContext* vctx = (VulkanContext*)Application::Get().GetWindow().GetGraphicsContext();

		vctx->GetVkContext().m_Swapchain.RecreateSwapchain(vctx);
	}

	void VulkanRenderAPI::DrawIndexed(const Ref<VertexArray>& vertexArray)
	{

	}

	void VulkanRenderAPI::Clear()
	{
		
	}
}
#pragma once
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#include "Engine/Renderer/RenderAPI.h"

namespace Kairos
{
	class VulkanRenderAPI : public RenderAPI
	{
	public:
		// Inherited via RenderAPI
		void Init() override;
		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		void Clear() override;
		void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) override;
	};
}
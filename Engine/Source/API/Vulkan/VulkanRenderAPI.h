#pragma once

#include "Engine/Renderer/RenderAPI.h"

namespace Kairos
{
	class VulkanRenderAPI : public RenderAPI
	{
		// Inherited via RenderAPI
		void Init() override;
		void SetClearColor(const glm::vec4& color) override;
		void Clear() override;
		void DrawIndexed(const Ref<VertexArray>& vertexArray) override;
	};
}
#pragma once

#include "Engine/Renderer/RHI/Framebuffer.h"

namespace Engine
{
	struct RenderPass
	{
		Framebuffer* TargetFramebuffer = nullptr;
		glm::vec4    ClearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
	};
}

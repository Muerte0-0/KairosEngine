#pragma once

#include <filesystem>

#include "GraphicsPipeline.h"

enum class API
{
	None = 0,
	Vulkan,
	DX11,
	DX12
};

namespace Engine
{
	class Framebuffer;

	class RenderAPI
	{
	public:
		virtual ~RenderAPI() = default;

		virtual void Init(void* windowHandle, const std::filesystem::path& shaderDirectory) = 0;
		
		virtual void BeginFrame() = 0;
		virtual void DrawFrame() = 0;
		virtual void EndFrame() = 0;
		
		virtual Framebuffer* GetFramebuffer() = 0;
		virtual GraphicsPipeline* GetGraphicsPipeline() = 0;
		
		virtual void WindowResized() = 0;
		virtual void ResizeFramebuffer(uint32_t width, uint32_t height) = 0;
	};
}

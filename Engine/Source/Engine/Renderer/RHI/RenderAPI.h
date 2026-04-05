#pragma once

#include <filesystem>

#include "GraphicsPipeline.h"
#include "Shader.h"

enum class API
{
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

		virtual void BeginScene() = 0;
		
		virtual void DrawFrame()  = 0;
		
		virtual void EndScene()   = 0;

		virtual Framebuffer*      GetFramebuffer()      = 0;
		virtual GraphicsPipeline* GetGraphicsPipeline() = 0;
		virtual ShaderLibrary*    GetShaderLibrary()    = 0;

		virtual void WindowResized()									= 0;
		virtual void ResizeFramebuffer(uint32_t width, uint32_t height)	= 0;
		
		virtual API GetType() = 0;
	};
}

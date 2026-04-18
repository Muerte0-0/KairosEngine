#pragma once

#include <filesystem>

#include "Buffer.h"
#include "Framebuffer.h"
#include "GraphicsPipeline.h"
#include "Resources/Mesh.h"
#include "Resources/Material.h"
#include "Shader.h"
#include "Engine/Renderer/RenderPass.h"
#include "Engine/Utils/RendererUtils.h"

enum class API
{
	Vulkan,
	DX11,
	DX12
};

namespace Engine
{
	class RenderAPI
	{
	public:
		virtual ~RenderAPI() = default;

		virtual void Init(void* windowHandle, const std::filesystem::path& shaderDirectory) = 0;

		virtual void BeginScene() = 0;
		virtual void BeginPass(const RenderPass& renderPass) = 0;
		virtual void EndPass() = 0;
		virtual void DrawMesh(
			const Framebuffer& framebuffer,
			const GraphicsPipeline& pipeline,
			const Mesh& mesh,
			const glm::mat4& modelTransform,
			const UniformBufferObject& uniformBufferObject,
			const std::vector<Ref<Material>>& materials = {}) = 0;
		virtual void DrawFrame()  = 0;
		virtual void EndScene()   = 0;

		virtual void WindowResized() = 0;

		virtual ShaderLibrary* GetShaderLibrary() = 0;
		virtual const std::filesystem::path& GetShaderDirectory() const = 0;
		virtual TextureFormat GetDefaultColorFormat() const = 0;
		virtual TextureFormat GetDefaultDepthFormat() const = 0;
		virtual void WaitIdle() = 0;

		// Release backend-side static/shared GPU resources before device destroy.
		// Called by Renderer::Shutdown() before WaitIdle().
		virtual void ReleaseStaticResources() = 0;

		virtual API GetType() = 0;

		// Material factory — implemented by each backend.
		virtual Ref<Material> CreateMaterial() = 0;
	};
}

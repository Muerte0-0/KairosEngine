#pragma once

#include "Camera.h"
#include "RenderPass.h"
#include "RHI/Framebuffer.h"
#include "RHI/GraphicsPipeline.h"
#include "RHI/Resources/Mesh.h"

#include "Engine/Core/Base.h"

#include <glm/glm.hpp>

namespace Engine
{
	struct SceneRendererSpecification
	{
		uint32_t        Width       = 1280;
		uint32_t        Height      = 720;
		TextureFormat   ColorFormat = TextureFormat::Undefined;
		TextureFormat   DepthFormat = TextureFormat::Undefined;
		SampleCountBits SampleCount = SampleCountBits::s1;
	};

	struct DrawCommand
	{
		Ref<Mesh> MeshRef;
		glm::mat4 Transform{ 1.0f };
	};

	class SceneRenderer
	{
	public:
		explicit SceneRenderer(const SceneRendererSpecification& spec = {});
		~SceneRenderer() = default;

		SceneRenderer(const SceneRenderer&)            = delete;
		SceneRenderer& operator=(const SceneRenderer&) = delete;

		void BeginScene(const Camera& camera);
		void SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform = glm::mat4(1.0f));
		void EndScene();

		void Resize(uint32_t width, uint32_t height);

		Framebuffer*      GetFramebuffer()      const { return m_Framebuffer.get(); }
		GraphicsPipeline* GetGraphicsPipeline() const { return m_Pipeline.get(); }

		uint32_t GetWidth() const { return m_Spec.Width; }
		uint32_t GetHeight() const { return m_Spec.Height; }

	private:
		void Flush();
		void CreateFramebuffer();
		void CreatePipeline(const BufferLayout& vertexLayout);
		void EnsureFormats();

		SceneRendererSpecification m_Spec;
		RenderPass                 m_RenderPass;
		Scope<Framebuffer>         m_Framebuffer;
		Scope<GraphicsPipeline>    m_Pipeline;

		bool                     m_SceneActive = false;
		glm::mat4                m_View{ 1.0f };
		glm::mat4                m_Projection{ 1.0f };
		std::vector<DrawCommand> m_DrawQueue;

		bool     m_ResizePending = false;
		uint32_t m_PendingWidth  = 0;
		uint32_t m_PendingHeight = 0;
	};
}

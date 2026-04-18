#include "kepch.h"
#include "SceneRenderer.h"

#include "Renderer.h"
#include "RHI/RenderAPI.h"
#include "RHI/Shader.h"

namespace Engine
{
	namespace
	{
		Ref<Shader> GetSceneShader()
		{
			ShaderLibrary* shaderLibrary = Renderer::GetShaderLibrary();
			ASSERT(shaderLibrary, "SceneRenderer: shader library is unavailable.");
			RenderAPI* api = Renderer::GetAPI();
			ASSERT(api, "SceneRenderer: RenderAPI is unavailable.");

			if (shaderLibrary->Exists("Mesh"))
				return shaderLibrary->Get("Mesh");

			ShaderDescriptor descriptor;
			descriptor.Name = "Mesh";
			descriptor.ShaderDirectory = api->GetShaderDirectory();
			descriptor.Stages = {
				{ ShaderStage::Vertex, "Mesh.vertMain.vert.spv", "vertMain" },
				{ ShaderStage::Fragment, "Mesh.fragMain.frag.spv", "fragMain" }
			};
			return shaderLibrary->Load(descriptor);
		}
	}

	SceneRenderer::SceneRenderer(const SceneRendererSpecification& spec)
		: m_Spec(spec)
	{
		m_RenderPass.ClearColor = { 0.025f, 0.025f, 0.025f, 1.0f };
		EnsureFormats();
		CreateFramebuffer();
		m_RenderPass.TargetFramebuffer = m_Framebuffer.get();
	}

	void SceneRenderer::BeginScene(const CameraManager& cameraManager)
	{
		ASSERT(!m_SceneActive, "SceneRenderer::BeginScene called while already active.");

		if (m_ResizePending)
		{
			RenderAPI* api = Renderer::GetAPI();
			ASSERT(api, "SceneRenderer::BeginScene requires a valid RenderAPI for resize.");
			api->WaitIdle();

			m_Spec.Width  = m_PendingWidth;
			m_Spec.Height = m_PendingHeight;
			m_Framebuffer->Resize(m_Spec.Width, m_Spec.Height);
			m_ResizePending = false;
		}

		const Camera* cam = cameraManager.GetActiveCamera();
		if (!cam) return;
		
		m_View       = cam->GetView();
		m_Projection = cam->GetProjection();
		m_DrawQueue.clear();
		m_SceneActive = true;
	}

	void SceneRenderer::SubmitMesh(const Ref<Mesh>& mesh,
	                               const glm::mat4& transform,
	                               std::vector<Ref<Material>> materials)
	{
		ASSERT(m_SceneActive, "SceneRenderer::SubmitMesh called outside BeginScene/EndScene.");
		ASSERT(mesh, "SceneRenderer::SubmitMesh called with a null mesh.");

		if (!m_Pipeline)
			CreatePipeline(mesh->GetLayout());

		m_DrawQueue.push_back({ mesh, std::move(materials), transform });
	}

	void SceneRenderer::EndScene()
	{
		ASSERT(m_SceneActive, "SceneRenderer::EndScene called without a matching BeginScene.");
		Flush();
		m_SceneActive = false;
	}

	void SceneRenderer::Resize(uint32_t width, uint32_t height)
	{
		width  = (std::max)(width, 1u);
		height = (std::max)(height, 1u);

		if (width == m_Spec.Width && height == m_Spec.Height)
			return;

		m_PendingWidth  = width;
		m_PendingHeight = height;
		m_ResizePending = true;
	}

	void SceneRenderer::Flush()
	{
		RenderAPI* api = Renderer::GetAPI();
		ASSERT(api, "SceneRenderer::Flush requires a valid RenderAPI.");

		m_RenderPass.TargetFramebuffer = m_Framebuffer.get();
		api->BeginPass(m_RenderPass);

		if (m_Pipeline != nullptr && !m_DrawQueue.empty())
		{
			for (DrawCommand& cmd : m_DrawQueue)
			{
				UniformBufferObject ubo{};
				ubo.View = m_View;
				ubo.Proj = m_Projection;
				api->DrawMesh(*m_Framebuffer, *m_Pipeline, *cmd.MeshRef,
				              cmd.Transform, ubo, cmd.Materials);
			}
		}

		api->EndPass();
	}

	void SceneRenderer::CreateFramebuffer()
	{
		FramebufferSpecification framebufferSpec;
		framebufferSpec.Width       = m_Spec.Width;
		framebufferSpec.Height      = m_Spec.Height;
		framebufferSpec.ColorFormat = m_Spec.ColorFormat;
		framebufferSpec.DepthFormat = m_Spec.DepthFormat;
		framebufferSpec.SampleCount = m_Spec.SampleCount;

		m_Framebuffer = Framebuffer::Create(framebufferSpec);
	}

	void SceneRenderer::CreatePipeline(const BufferLayout& vertexLayout)
	{
		GraphicsPipelineCreateInfo createInfo;
		createInfo.Shader       = GetSceneShader();
		createInfo.VertexLayout = vertexLayout;
		createInfo.ColorFormat  = m_Spec.ColorFormat;
		createInfo.DepthFormat  = m_Spec.DepthFormat;
		createInfo.SampleCount  = m_Spec.SampleCount;

		m_Pipeline = GraphicsPipeline::Create(std::move(createInfo));
	}

	void SceneRenderer::EnsureFormats()
	{
		RenderAPI* api = Renderer::GetAPI();
		ASSERT(api, "SceneRenderer::EnsureFormats requires a valid RenderAPI.");

		if (m_Spec.ColorFormat != TextureFormat::Undefined &&
			m_Spec.DepthFormat != TextureFormat::Undefined)
		{
			return;
		}

		if (m_Spec.ColorFormat == TextureFormat::Undefined)
			m_Spec.ColorFormat = api->GetDefaultColorFormat();

		if (m_Spec.DepthFormat == TextureFormat::Undefined)
			m_Spec.DepthFormat = api->GetDefaultDepthFormat();
	}
}

#include "kepch.h"
#include "SceneRenderer.h"

#include "Renderer.h"
#include "RenderPipeline.h"
#include "RHI/RenderAPI.h"
#include "RHI/Shader.h"
#include "Engine/Scene/Systems/MeshRenderSystem.h"

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
		m_RenderPipeline = CreateScope<RenderPipeline>(*this);
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
		m_ShadowData = {};
		m_DrawQueue.clear();
		m_LightQueue.clear();
		m_SceneActive = true;
	}

	void SceneRenderer::Render(entt::registry& registry)
	{
		ASSERT(m_SceneActive, "SceneRenderer::Render called outside BeginScene/EndScene.");
		ASSERT(m_RenderPipeline, "SceneRenderer::Render requires a valid render pipeline.");
		m_RenderPipeline->Execute(registry);
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

	void SceneRenderer::SubmitLight(const LightSubmission& light)
	{
		ASSERT(m_SceneActive, "SceneRenderer::SubmitLight called outside BeginScene/EndScene.");
		m_LightQueue.push_back(light);
	}

	void SceneRenderer::EndScene()
	{
		ASSERT(m_SceneActive, "SceneRenderer::EndScene called without a matching BeginScene.");
		m_SceneActive = false;
	}

	void SceneRenderer::SetShadowData(const ShadowSceneData& shadowData)
	{
		m_ShadowData = shadowData;
	}

	void SceneRenderer::ExecuteGeometryPass(entt::registry& registry)
	{
		MeshRenderSystem::Render(registry, *this);
		Flush();
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
		ASSERT(api, "SceneRenderer::Flush requires a valid RenderAPI.")

		if (m_Pipeline)
			api->PrepareForDraw(*m_Pipeline);
		
		m_RenderPass.TargetFramebuffer = m_Framebuffer.get();
		api->BeginPass(m_RenderPass);

		if (m_Pipeline != nullptr && !m_DrawQueue.empty())
		{
			// Build SceneData — pack lights from submission queue
			SceneData sceneData{};
			sceneData.View = m_View;
			sceneData.Proj = m_Projection;
			sceneData.LightViewProj = m_ShadowData.LightViewProj;
			sceneData.ShadowParams = glm::vec4(m_ShadowData.Bias, m_ShadowData.TexelSize, 0.0f, 0.0f);
			sceneData.LightCount = static_cast<int>(
				std::min(static_cast<size_t>(MAX_LIGHTS), m_LightQueue.size()));
			sceneData.ShadowEnabled = m_ShadowData.Enabled ? 1 : 0;
			sceneData.ShadowLightIndex = -1;

			for (int i = 0; i < sceneData.LightCount; ++i)
			{
				const LightSubmission& src = m_LightQueue[i];
				GpuLight& dst = sceneData.Lights[i];

				dst.Position        = glm::vec4(src.Position,  1.f);
				dst.Direction       = glm::vec4(glm::normalize(src.Direction), 0.f);
				dst.ColorIntensity  = glm::vec4(src.Color, src.Intensity);
				dst.Range           = src.Range;
				dst.InnerConeAngle  = src.InnerConeAngle;
				dst.OuterConeAngle  = src.OuterConeAngle;
				dst.Type            = src.Type;

				if (src.EntityID == static_cast<uint32_t>(m_ShadowData.LightEntityID))
					sceneData.ShadowLightIndex = i;
			}

			api->SetShadowMap(sceneData.ShadowEnabled ? m_RenderPipeline->GetShadowMapFramebuffer() : nullptr);

			for (DrawCommand& cmd : m_DrawQueue)
			{
				api->DrawMesh(*m_Framebuffer, *m_Pipeline, *cmd.MeshRef,
				              cmd.Transform, sceneData, cmd.Materials);
			}
		}
		else
		{
			api->SetShadowMap(nullptr);
		}

		api->EndPass();
		api->SetShadowMap(nullptr);
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

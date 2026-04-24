#include "kepch.h"
#include "ShadowPass.h"

#include "Renderer.h"
#include "RHI/RenderAPI.h"
#include "RHI/Shader.h"
#include "Engine/Scene/Systems/ShadowRenderSystem.h"

namespace Engine
{
	namespace
	{
		Ref<Shader> GetShadowShader()
		{
			ShaderLibrary* shaderLibrary = Renderer::GetShaderLibrary();
			ASSERT(shaderLibrary, "ShadowPass: shader library is unavailable.");
			RenderAPI* api = Renderer::GetAPI();
			ASSERT(api, "ShadowPass: RenderAPI is unavailable.");

			if (shaderLibrary->Exists("ShadowMap"))
				return shaderLibrary->Get("ShadowMap");

			ShaderDescriptor descriptor;
			descriptor.Name = "ShadowMap";
			descriptor.ShaderDirectory = api->GetShaderDirectory();
			descriptor.Stages = {
				{ ShaderStage::Vertex, "ShadowMap.vertMain.vert.spv", "vertMain" }
			};
			return shaderLibrary->Load(descriptor);
		}
	}

	void ShadowPass::Init()
	{
		CreateFramebuffer();
		m_RenderPass.TargetFramebuffer = m_Framebuffer.get();
		m_RenderPass.ClearColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	}

	void ShadowPass::Begin()
	{
		m_LightData = {};
	}

	ShadowSceneData ShadowPass::Render(entt::registry& registry)
	{
		m_LightData = ShadowRenderSystem::ExtractDirectionalLight(registry, SHADOW_MAP_SIZE);
		if (m_LightData.Valid)
		{
			RenderAPI* api = Renderer::GetAPI();
			ASSERT(api, "ShadowPass::Render requires a valid RenderAPI.");
			api->BeginPass(m_RenderPass);
			ShadowRenderSystem::Render(registry, *this, m_LightData);
		}

		ShadowSceneData shadowData;
		shadowData.LightViewProj = m_LightData.LightViewProjection;
		shadowData.Bias = m_LightData.Bias;
		shadowData.TexelSize = m_LightData.TexelSize;
		shadowData.LightEntityID = m_LightData.LightEntityID;
		shadowData.Enabled = m_LightData.Valid;
		return shadowData;
	}

	void ShadowPass::End()
	{
		if (!m_LightData.Valid)
			return;

		RenderAPI* api = Renderer::GetAPI();
		ASSERT(api, "ShadowPass::End requires a valid RenderAPI.")
		api->EndPass();
	}

	void ShadowPass::SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& modelTransform, const glm::mat4& lightViewProjection)
	{
		ASSERT(mesh, "ShadowPass::SubmitMesh called with a null mesh.");
		if (!m_Pipeline)
			CreatePipeline(mesh->GetLayout());

		SceneData sceneData{};
		sceneData.View = glm::mat4(1.0f);
		sceneData.Proj = lightViewProjection;
		sceneData.LightViewProj = lightViewProjection;

		RenderAPI* api = Renderer::GetAPI();
		ASSERT(api, "ShadowPass::SubmitMesh requires a valid RenderAPI.");
		api->DrawMesh(*m_Framebuffer, *m_Pipeline, *mesh, modelTransform, sceneData, {});
	}

	void ShadowPass::CreateFramebuffer()
	{
		FramebufferSpecification framebufferSpec;
		framebufferSpec.Width = SHADOW_MAP_SIZE;
		framebufferSpec.Height = SHADOW_MAP_SIZE;
		framebufferSpec.ColorFormat = TextureFormat::Undefined;
		framebufferSpec.DepthFormat = TextureFormat::Depth32_Float;
		framebufferSpec.SampleCount = SampleCountBits::s1;

		m_Framebuffer = Framebuffer::Create(framebufferSpec);
	}

	void ShadowPass::CreatePipeline(const BufferLayout& vertexLayout)
	{
		GraphicsPipelineCreateInfo createInfo;
		createInfo.Shader = GetShadowShader();
		createInfo.VertexLayout = vertexLayout;
		createInfo.ColorFormat = TextureFormat::Undefined;
		createInfo.DepthFormat = TextureFormat::Depth32_Float;
		createInfo.SampleCount = SampleCountBits::s1;

		m_Pipeline = GraphicsPipeline::Create(std::move(createInfo));
	}
}

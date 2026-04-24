#pragma once

#include "RenderPipeline.h"
#include "RenderPass.h"
#include "RHI/Framebuffer.h"
#include "RHI/GraphicsPipeline.h"
#include "RHI/Resources/Mesh.h"
#include "RHI/Resources/Material.h"

#include "Engine/Core/Base.h"
#include "entt.hpp"

#include <glm/glm.hpp>

#include "Cameras/CameraManager.h"

namespace Engine
{
	struct SceneRendererSpecification
	{
		uint32_t        Width       = 1280;
		uint32_t        Height      = 720;
		TextureFormat   ColorFormat = TextureFormat::Undefined;
		TextureFormat   DepthFormat = TextureFormat::Undefined;
		SampleCountBits SampleCount = SampleCountBits::s4;
	};

	struct DrawCommand
	{
		Ref<Mesh>                  MeshRef;
		std::vector<Ref<Material>> Materials;   // indexed by SubMesh::MaterialIndex
		glm::mat4                  Transform{ 1.0f };
	};

	struct LightSubmission
	{
		uint32_t  EntityID = 0;
		glm::vec3 Position;
		glm::vec3 Direction;
		glm::vec3 Color;
		float     Intensity;
		float     Range;
		float     InnerConeAngle;   // cosine
		float     OuterConeAngle;   // cosine
		int       Type;             // 0=Directional 1=Point 2=Spot
	};

	struct ShadowSceneData
	{
		glm::mat4 LightViewProj{ 1.0f };
		float     Bias = 0.0f;
		float     TexelSize = 0.0f;
		int       LightEntityID = -1;
		bool      Enabled = false;
	};
	class SceneRenderer
	{
	public:
		explicit SceneRenderer(const SceneRendererSpecification& spec = {});
		~SceneRenderer() = default;

		SceneRenderer(const SceneRenderer&)            = delete;
		SceneRenderer& operator=(const SceneRenderer&) = delete;

		void BeginScene(const CameraManager& cameraManager);
		void Render(entt::registry& registry);
		void SubmitMesh(const Ref<Mesh>& mesh,
		                const glm::mat4& transform = glm::mat4(1.0f),
		                std::vector<Ref<Material>> materials = {});
		void SubmitLight(const LightSubmission& light);
		void EndScene();

		void Resize(uint32_t width, uint32_t height);

		Framebuffer*      GetFramebuffer()      const { return m_Framebuffer.get(); }
		GraphicsPipeline* GetGraphicsPipeline() const { return m_Pipeline.get(); }

		uint32_t GetWidth() const { return m_Spec.Width; }
		uint32_t GetHeight() const { return m_Spec.Height; }

		void SetShadowData(const ShadowSceneData& shadowData);
		void ExecuteGeometryPass(entt::registry& registry);

	private:
		void Flush();
		void CreateFramebuffer();
		void CreatePipeline(const BufferLayout& vertexLayout);
		void EnsureFormats();

		SceneRendererSpecification m_Spec;
		RenderPass                 m_RenderPass;
		Scope<Framebuffer>         m_Framebuffer;
		Scope<GraphicsPipeline>    m_Pipeline;

		bool                       m_SceneActive = false;
		glm::mat4                  m_View{ 1.0f };
		glm::mat4                  m_Projection{ 1.0f };
		ShadowSceneData            m_ShadowData;
		std::vector<DrawCommand>   m_DrawQueue;
		std::vector<LightSubmission> m_LightQueue;
		Scope<RenderPipeline>      m_RenderPipeline;

		bool     m_ResizePending = false;
		uint32_t m_PendingWidth  = 0;
		uint32_t m_PendingHeight = 0;
	};
}

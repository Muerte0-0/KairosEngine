#include "kepch.h"
#include "MeshRenderSystem.h"

#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Scene/Components.h"

namespace Engine::MeshRenderSystem
{
	void Render(entt::registry& registry, SceneRenderer& renderer)
	{
		auto view = registry.view<MeshComponent, TransformComponent>();
		
		for (auto entity : view)
		{
			const auto& [meshComponent, transformComponent] = view.get<MeshComponent, TransformComponent>(entity);
			
			if (meshComponent.HasMesh())
				renderer.SubmitMesh(meshComponent.Mesh, transformComponent.GetTransform());
		}
	}
}

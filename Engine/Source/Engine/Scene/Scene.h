#pragma once

#include "entt.hpp"

namespace Kairos
{
	class SceneHierarchyPanel;
}

namespace Engine
{
	class Entity;
	class SceneSerializer;
	class SceneRenderer;

	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = "Empty Entity");
		void DestroyEntity(Entity entity);

		void OnUpdate(float deltaTime);

		/**
		 * @brief Walk all MeshComponent + TransformComponent entities and
		 *        submit them to the provided SceneRenderer.
		 *        Call this between SceneRenderer::BeginScene and EndScene.
		 */
		void OnRender(SceneRenderer& renderer);

	private:
		entt::registry m_Registry;
		
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		friend class Entity;
		friend class SceneSerializer;
		friend class Kairos::SceneHierarchyPanel;
	};
}

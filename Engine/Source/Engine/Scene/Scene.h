#pragma once

#include "Engine/Core/UUID.h"

#include "entt.hpp"
#include <functional>

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
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = "Empty Entity");
		void DestroyEntity(Entity entity);

		void OnUpdate(float deltaTime);

		const std::string& GetName() const { return m_Name; }
		void               SetName(const std::string& name) { m_Name = name; }

		/**
		 * @brief Walk all MeshComponent + TransformComponent entities and
		 *        submit them to the provided SceneRenderer.
		 *        Call this between SceneRenderer::BeginScene and EndScene.
		 */
		void OnRender(SceneRenderer& renderer);

		/**
		 * @brief Invoke fn(entity) for every living entity in the scene.
		 *        Provides iteration without exposing entt to callers.
		 */
		void EachEntity(const std::function<void(Entity)>& fn);

	private:
		entt::registry m_Registry;
		std::string    m_Name = "Untitled";
		
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		friend class Entity;
		friend class SceneSerializer;
		friend class Kairos::SceneHierarchyPanel;
	};
}

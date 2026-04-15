#include "kepch.h"
#include "Scene.h"

#include "Components.h"
#include "Entity.h"
#include "Systems/MeshRenderSystem.h"

namespace Engine
{
	Scene::Scene()
	{
		
	}

	Scene::~Scene()
	{
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name;
		
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::OnUpdate(float deltaTime)
	{
	}

	void Scene::OnRender(SceneRenderer& renderer)
	{
		MeshRenderSystem::Render(m_Registry, renderer);
	}

	void Scene::EachEntity(const std::function<void(Entity)>& fn)
	{
		for (auto entity : std::views::reverse(m_Registry.view<entt::entity>()))
			fn(Entity{ entity, this });
	}

	template <typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		
	}
	
	template <>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{
		
	}
	
	template <>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
		
	}
	
	template <>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
		
	}
	
	template <>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		
	}
	
	template <>
	void Scene::OnComponentAdded<MeshComponent>(Entity entity, MeshComponent& component)
	{
		
	}
}

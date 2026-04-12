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
		Entity entity = { m_Registry.create(), this };
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
}

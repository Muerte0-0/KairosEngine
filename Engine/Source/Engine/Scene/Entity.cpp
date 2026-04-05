#include "kepch.h"
#include "Entity.h"

namespace Engine
{
	Entity::Entity(entt::entity entity, Scene* scene) : m_EntityHandle(entity), m_Scene(scene)
	{}
}

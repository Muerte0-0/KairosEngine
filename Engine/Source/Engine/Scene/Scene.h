#pragma once

#include "entt.hpp"

namespace Engine
{
	class Entity;
	
	class Scene
	{
	public:
		Scene();
		~Scene();
		
		Entity CreateEntity(const std::string& name = "Empty Entity");
		
		void OnUpdate(float deltaTime);
		
	private:
		entt::registry m_Registry;
		
		friend class Entity;
	};
}

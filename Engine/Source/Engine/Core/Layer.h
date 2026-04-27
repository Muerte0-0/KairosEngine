#pragma once
#include "Base.h"
#include <Engine/Events/Event.h>

namespace Engine
{
	class Scene;

	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;
		
		virtual void OnAttach() {}
		virtual void OnDetach() {}
		
		virtual void OnEvent(Event& event) {}
		
		virtual void OnUpdate(float deltaTime) {}
		virtual void OnFixedUpdate(float deltaTime) {}
		virtual void OnRender() {}
		virtual void OnImGuiRender() {}

		// Called by Application on the main thread after a scene transition completes.
		// Override in EditorLayer (or game layer) to swap the active scene.
		virtual void OnSceneLoaded(const Ref<Scene>& scene) {}
		
	private:
		std::string m_DebugName;
	};
}
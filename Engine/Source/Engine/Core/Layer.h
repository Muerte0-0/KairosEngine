#pragma once
#include <Engine/Events/Event.h>

namespace Engine
{
	class Layer
	{
	public:
		virtual ~Layer() = default;
		
		virtual void OnAttach() {}
		virtual void OnDetach() {}
		
		virtual void OnEvent(Event& event) {}
		
		virtual void OnUpdate(float deltaTime) {}
		virtual void OnFixedUpdate(float deltaTime) {}
		virtual void OnRender() {}
		virtual void OnImGuiRender() {}
	};
}
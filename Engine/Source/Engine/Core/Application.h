#pragma once
#include "Layer.h"
#include "Window.h"
#include "Engine/Events/Event.h"
#include "Engine/Events/WindowEvents.h"

#include <glm/glm.hpp>

namespace Engine
{
	struct ApplicationSpecification
	{
		std::string Name = "Application";
		WindowSpecification WindowSpec;
	};
	
	class Application
	{
	public:
		Application();
		~Application();
		
		void Initialize();
		void Shutdown();
		void Run();
		void Stop();
		
		void RaiseEvent(Event& event);
		
		template<typename TLayer>
		requires is_base_of_v<Layer, TLayer>
		void PushLayer()
		{ m_LayerStack.push_back(CreateScope<Layer>()); }
		
		template<typename TLayer>
		requires is_base_of_v<Layer, TLayer>
		TLayer* GetLayer()
		{
			for (const auto& layer : m_LayerStack)
			{
				if (auto casted = dynamic_cast<TLayer*>(layer.get()))
					return casted;
			}
			
			return nullptr;
		}
		
		glm::vec2 GetFramebufferSize() const;

		Ref<Window> GetWindow() const { return m_Window; }

		static Application& Get();
		static float GetTime();
		
		virtual ApplicationSpecification GetApplicationSpecs() = 0;
		
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnWindowClosed(WindowClosedEvent& e);
		
	private:
		Ref<Window> m_Window;

		std::vector<Scope<Layer>> m_LayerStack;
		
		bool m_IsMinimized = false;
		
		friend class Layer;
	};
	
	Application* CreateApplication(int argc, char** argv);
}

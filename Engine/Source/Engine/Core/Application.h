#pragma once
#include "Base.h"

#include "Layer.h"
#include "Window.h"
#include "Engine/Events/Event.h"
#include "Engine/Events/WindowEvents.h"

#include <glm/glm.hpp>

#include "Engine/ImGui/ImGuiLayer.h"

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
		virtual ~Application();
		
		void Initialize();
		void Shutdown();
		void Run();
		void Stop();
		
		void RaiseEvent(Event& event);
		
		template<typename TLayer>
		requires(std::is_base_of_v<Layer, TLayer>)
		void PushLayer()
		{
			m_LayerStack.push_back(CreateScope<TLayer>());
		}

		template<typename TLayer>
		requires(std::is_base_of_v<Layer, TLayer>)
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
		Scope<ImGuiLayer> m_ImGuiLayer;
		
		bool m_IsMinimized = false;
		
		friend class Layer;
	};
	
	Application* CreateApplication(int argc, char** argv);
}

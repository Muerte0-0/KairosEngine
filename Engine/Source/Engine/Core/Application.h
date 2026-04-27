#pragma once
#include "Base.h"

#include "Layer.h"
#include "Window.h"
#include "EngineState.h"
#include "LoadingSystem.h"
#include "Engine/Events/Event.h"
#include "Engine/Events/WindowEvents.h"
#include "Engine/ImGui/LoadingScreen.h"

#include <filesystem>
#include <glm/glm.hpp>

#include "Engine/ImGui/ImGuiLayer.h"
#include "Engine/ImGui/ImGuiUtils.h"

namespace Engine
{
	struct ApplicationSpecification
	{
		std::string Name = "Application";
		
		Theme Theme = Theme::Mocha;
		
		WindowSpecification WindowSpec;
		
		std::filesystem::path ShaderSourcePath;
		bool CompileShadersOnStartup = false;
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

		// Loading system integration
		// Call to trigger an async scene transition (sets state to Loading).
		void RequestSceneChange(const std::string& path);

		EngineState   GetEngineState()    const { return m_EngineState; }
		LoadingPhase  GetLoadingPhase()   const { return m_LoadingPhase; }
		
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
		
		virtual ApplicationSpecification GetApplicationSpecs() const = 0;
		
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnWindowClosed(WindowClosedEvent& e);

	private:
		void EnsureApplicationShadersCompiled() const;
		void RenderLoadingScreen();
		void TickLoadingState(float deltaTime);
		
		Ref<Window> m_Window;

		std::vector<Scope<Layer>> m_LayerStack;
		Scope<ImGuiLayer> m_ImGuiLayer;

		// Engine state
		EngineState  m_EngineState  = EngineState::Loading;
		LoadingPhase m_LoadingPhase = LoadingPhase::EditorStartup;
		LoadingScreen m_LoadingScreen;
		
		bool m_IsMinimized = false;
		
		friend class Layer;
	};
	
	Application* CreateApplication(int argc, char** argv);
}

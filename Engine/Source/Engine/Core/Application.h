#pragma once
#include "Base.h"

#include "Layer.h"
#include "Window.h"
#include "EngineState.h"
#include "LoadingSystem.h"
#include "Threading/FramePipeline.h"
#include "Engine/Events/Event.h"
#include "Engine/Events/WindowEvents.h"
#include "Engine/ImGui/LoadingScreen.h"

#include <filesystem>
#include <glm/glm.hpp>

#include "Engine/ImGui/ImGuiLayer.h"
#include "Engine/ImGui/ImGuiUtils.h"
#include "Engine/Scene/Scene.h"

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
		EngineMode    GetEngineMode()     const { return m_EngineMode; }
		void          SetEngineMode(EngineMode mode) { m_EngineMode = mode; }

		// Play mode transitions. Must be called on main thread.
		void EnterPlayMode();
		void ExitPlayMode();

		// Called by EditorLayer to register the current editor scene so
		// EnterPlayMode can clone it.
		void SetEditorScene(const Ref<Scene>& scene) { m_EditorScene = scene; }
		
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
		void RenderGenericOverlay();
		void TickLoadingState(float deltaTime);
		void AttachLayersDuringLoading();
		
		Ref<Window> m_Window;

		std::vector<Scope<Layer>> m_LayerStack;
		Scope<ImGuiLayer> m_ImGuiLayer;

		// Engine state
		EngineState  m_EngineState  = EngineState::Loading;
		LoadingPhase m_LoadingPhase = LoadingPhase::EditorStartup;
		EngineMode   m_EngineMode   = EngineMode::Editor;
		LoadingScreen m_LoadingScreen;

		// Play mode scene management (main thread only)
		Ref<Scene> m_EditorScene{ nullptr };
		Ref<Scene> m_RuntimeScene{ nullptr };
		
		bool m_IsMinimized = false;

		// Deferred layer attach so first loading frames can render.
		bool   m_LayersAttached     = false;
		size_t m_NextLayerToAttach  = 0;
		bool   m_LayerAttachPrimed  = false;

		// Scene transition: keep scene in loading until assets resolved.
		Ref<Scene> m_PendingSceneTransition{ nullptr };
		bool       m_SceneTransitionResolveStarted = false;
		bool       m_SceneTransitionPrimed = false;

		// CPU frame pipeline — dispatches ECS/Simulation/RenderExtract/CommandBuild
		// stages to JobSystem workers each frame. GPU submit stays on main thread.
		FramePipeline m_FramePipeline;
		
		friend class Layer;
		// Allow loading worker to trigger CPU-only shader compilation without
		// exposing this startup detail on the public API.
		friend class LoadingSystem;
	};
	
	Application* CreateApplication(int argc, char** argv);
}

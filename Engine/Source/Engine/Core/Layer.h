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

		// Called by FramePipeline on a worker thread — NO Vulkan calls allowed.
		// Override to run physics, animation, or other simulation work.
		virtual void OnSimulate(float deltaTime) {}

		// Called by FramePipeline on a worker thread after OnUpdate — NO Vulkan calls.
		// Override to snapshot transforms, run visibility culling, build render lists.
		virtual void OnRenderExtract() {}

		virtual void OnRender() {}
		virtual void OnImGuiRender() {}

		// Called by Application on the main thread after a scene transition completes.
		// Override in EditorLayer (or game layer) to swap the active scene.
		virtual void OnSceneLoaded(const Ref<Scene>& scene) {}

		// Called by Application when entering/exiting play mode.
		virtual void OnEnterPlayMode(const Ref<Scene>& runtimeScene) {}
		virtual void OnExitPlayMode(const Ref<Scene>& editorScene) {}
		
	private:
		std::string m_DebugName;
	};
}
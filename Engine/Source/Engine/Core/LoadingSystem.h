#pragma once

#include <atomic>
#include <string>
#include <memory>
#include <thread>
#include <functional>

#include "Engine/Core/Base.h"

namespace Engine
{
	class Scene;

	// -----------------------------------------------------------------------
	// Holds state for an async scene load. Lives as a static inside
	// LoadingSystem. Thread-safe via atomics; Ref swap on main thread only.
	// -----------------------------------------------------------------------
	struct SceneLoadContext
	{
		std::string TargetPath;

		std::atomic<float> Progress{ 0.0f };
		std::atomic<bool>  IsDone{ true };  // true = "not loading"; set false when load begins
		std::atomic<bool>  HasError{ false };
		std::string        ErrorMessage;

		Ref<Scene> LoadedScene{ nullptr };
	};

	// -----------------------------------------------------------------------
	// LoadingSystem
	// Central async coordinator. Manages startup init and scene transitions.
	// No Vulkan calls on worker threads — GPU work deferred to Finalize*().
	// -----------------------------------------------------------------------
	class LoadingSystem
	{
	public:
		// ----- Startup loading -----

		// Kick off async startup (asset registry, shader validation, etc.)
		// Optional per-step status callback (called from worker thread — no GPU!).
		static void StartStartupLoading(std::function<void(const std::string&)> statusCallback = nullptr);
		static bool IsStartupDone();
		static float GetStartupProgress();
		static const std::string& GetCurrentStatusText();

		// Called on main thread once IsStartupDone() == true.
		// Joins worker thread; safe to make GPU calls after.
		static void FinalizeStartup();

		// ----- Scene transition loading -----

		static void LoadSceneAsync(const std::string& path);
		static bool IsSceneLoadDone();
		static float GetSceneLoadProgress();

		// Called on main thread once IsSceneLoadDone() == true.
		// Returns the loaded Scene (ownership transferred to caller).
		static Ref<Scene> FinalizeSceneLoad();

		// ----- Generic progress query (works for both phases) -----
		static float GetProgress();
		static bool  IsLoading();

		static void Reset();

	private:
		static SceneLoadContext    s_SceneCtx;
		static std::atomic<float>  s_StartupProgress;
		static std::atomic<bool>   s_StartupDone;
		static std::string         s_StatusText;
		static std::thread         s_StartupThread;
		static std::thread         s_SceneThread;

		static std::function<void(const std::string&)> s_StatusCallback;

		static void SetStatus(const std::string& text);
		static void StartupWorker();
		static void SceneWorker(const std::string& path);
	};
}

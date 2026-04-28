#pragma once

#include <atomic>
#include <string>
#include <memory>
#include <mutex>
#include <functional>
#include <vector>

#include "Engine/Assets/Asset.h"
#include "Engine/Core/Base.h"
#include "Engine/Core/Threading/JobSystem.h"

namespace Engine
{
	enum class LoadingMode
	{
		Fullscreen,
		Overlay
	};

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

		// Collected during async deserialize (worker thread). Used later on main thread
		// to resolve assets incrementally while keeping loading UI responsive.
		std::vector<AssetHandle> MeshAssets;
		std::vector<std::string> Primitives;
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
		static std::string GetCurrentStatusText();
		// Main-thread helpers to keep loading UI accurate while doing deferred work
		// (e.g. layer OnAttach) during startup.
		static void SetStartupProgressMainThread(float progress);
		static void SetStatusTextMainThread(const std::string& text);

		// Startup scene asset resolution (main thread, incremental).
		// Scene deserialization enqueues referenced assets without loading them.
		static void SetStartupScene(const Ref<Scene>& scene);
		static void EnqueueStartupMeshAsset(AssetHandle handle);
		static void EnqueueStartupPrimitive(const std::string& key);
		static bool PumpStartupMainThreadWork(uint32_t meshBudgetPerFrame = 1, uint32_t primBudgetPerFrame = 1);

		// Called on main thread once IsStartupDone() == true.
		// Joins worker thread; safe to make GPU calls after.
		static void FinalizeStartup();

		// ----- Scene transition loading -----

		static void LoadSceneAsync(const std::string& path);
		static bool IsSceneLoadDone();
		static float GetSceneLoadProgress();
		static void StartSceneTransitionResolve(const Ref<Scene>& scene);
		static bool PumpSceneTransitionMainThreadWork(uint32_t meshBudgetPerFrame = 1, uint32_t primBudgetPerFrame = 1);

		// Called on main thread once IsSceneLoadDone() == true.
		// Returns the loaded Scene (ownership transferred to caller).
		static Ref<Scene> FinalizeSceneLoad();

		// ----- Generic progress query (works for both phases) -----
		static float GetProgress();
		static bool  IsLoading();
		static LoadingMode GetLoadingMode();

		// ----- Generic Manual Loading API -----
		static void BeginLoading(LoadingMode mode);
		static void EndLoading();
		static void SetLoadingText(const std::string& text);
		static void SetProgress(float progress);

		static void Reset();

	private:
		static SceneLoadContext    s_SceneCtx;
		static std::atomic<float>  s_StartupProgress;
		static std::atomic<bool>   s_StartupDone;
		static std::string         s_StatusText;
		static JobHandle           s_StartupJob;
		static JobHandle           s_SceneJob;

		static LoadingMode         s_CurrentMode;
		static std::atomic<bool>   s_GenericIsLoading;
		static std::atomic<float>  s_GenericProgress;

		static std::function<void(const std::string&)> s_StatusCallback;
		static std::mutex          s_StatusMutex;

		// Startup incremental work (main thread only).
		static Ref<Scene>          s_StartupScene;
		static std::vector<AssetHandle> s_StartupMeshQueue;
		static std::vector<std::string> s_StartupPrimitiveQueue;
		static size_t             s_StartupMeshIndex;
		static size_t             s_StartupPrimIndex;

		static void SetStatus(const std::string& text);
		static void StartupWorker();
		static void SceneWorker(const std::string& path);

		// Shared enqueue hook used by SceneSerializer.
		// If a scene context is active on this thread, enqueue into it; otherwise enqueue into startup queues.
		static void EnqueueMesh(AssetHandle handle);
		static void EnqueuePrimitive(const std::string& key);

		// Scene transition resolve (main thread)
		static Ref<Scene>          s_SceneResolveScene;
		static std::vector<AssetHandle> s_SceneResolveMeshQueue;
		static std::vector<std::string> s_SceneResolvePrimitiveQueue;
		static size_t             s_SceneResolveMeshIndex;
		static size_t             s_SceneResolvePrimIndex;
		static std::atomic<float> s_SceneResolveProgress;

		// Thread-local enqueue target for SceneSerializer during async deserialize
		static thread_local SceneLoadContext* t_EnqueueSceneCtx;
	};
}

#include "kepch.h"
#include "LoadingSystem.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneSerializer.h"
#include "Engine/Assets/AssetRegistry.h"
#include "Engine/Debugging/Log.h"

namespace Engine
{
	// -----------------------------------------------------------------------
	// Static storage
	// -----------------------------------------------------------------------
	SceneLoadContext    LoadingSystem::s_SceneCtx;
	std::atomic<float>  LoadingSystem::s_StartupProgress{ 0.0f };
	std::atomic<bool>   LoadingSystem::s_StartupDone{ false };
	std::string         LoadingSystem::s_StatusText = "Initializing...";
	std::thread         LoadingSystem::s_StartupThread;
	std::thread         LoadingSystem::s_SceneThread;
	std::function<void(const std::string&)> LoadingSystem::s_StatusCallback;

	// -----------------------------------------------------------------------
	// Internal helpers
	// -----------------------------------------------------------------------
	void LoadingSystem::SetStatus(const std::string& text)
	{
		s_StatusText = text;
		if (s_StatusCallback)
			s_StatusCallback(text);
	}

	// -----------------------------------------------------------------------
	// Startup loading
	// -----------------------------------------------------------------------
	void LoadingSystem::StartupWorker()
	{
		// 4 logical steps — each contributes 25% progress.
		// IMPORTANT: No Vulkan calls here. GPU work happens in FinalizeStartup().

		SetStatus("Scanning asset registry...");
		// TODO: call AssetRegistry::Scan() or equivalent when available
		std::this_thread::sleep_for(std::chrono::milliseconds(10)); // yield
		s_StartupProgress.store(0.25f);

		SetStatus("Validating shaders...");
		// TODO: verify compiled shader artefacts exist on disk
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		s_StartupProgress.store(0.50f);

		SetStatus("Loading default resources...");
		// TODO: preload default textures / meshes (CPU side only)
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		s_StartupProgress.store(0.75f);

		SetStatus("Preparing editor systems...");
		// TODO: any editor-specific CPU init
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		s_StartupProgress.store(1.0f);

		SetStatus("Ready.");
		s_StartupDone.store(true);
	}

	void LoadingSystem::StartStartupLoading(std::function<void(const std::string&)> statusCallback)
	{
		s_StatusCallback   = std::move(statusCallback);
		s_StartupProgress.store(0.0f);
		s_StartupDone.store(false);
		s_StatusText = "Initializing Engine...";

		s_StartupThread = std::thread(&LoadingSystem::StartupWorker);
	}

	bool  LoadingSystem::IsStartupDone()       { return s_StartupDone.load(); }
	float LoadingSystem::GetStartupProgress()  { return s_StartupProgress.load(); }
	const std::string& LoadingSystem::GetCurrentStatusText() { return s_StatusText; }

	void LoadingSystem::FinalizeStartup()
	{
		ASSERT(s_StartupDone.load(), "FinalizeStartup called before startup worker finished");
		if (s_StartupThread.joinable())
			s_StartupThread.join();
		// GPU-side init (pipeline warm-up, fallback texture upload, etc.) goes here.
		LOG(LogLevel::Info, "Engine startup finalized.");
	}

	// -----------------------------------------------------------------------
	// Scene transition loading
	// -----------------------------------------------------------------------
	void LoadingSystem::SceneWorker(const std::string& path)
	{
		// CPU-only: deserialize YAML into an in-memory Scene.
		// No Vulkan calls. GPU mesh/texture upload deferred to FinalizeSceneLoad().
		s_SceneCtx.Progress.store(0.1f);

		auto scene = CreateRef<Scene>();
		SceneSerializer serializer(scene);

		s_SceneCtx.Progress.store(0.4f);

		const bool ok = serializer.Deserialize(path);
		if (!ok)
		{
			s_SceneCtx.HasError.store(true);
			s_SceneCtx.ErrorMessage = "Failed to deserialize scene: " + path;
			LOG(LogLevel::Error, "{}", s_SceneCtx.ErrorMessage);
			s_SceneCtx.Progress.store(1.0f);
			s_SceneCtx.IsDone.store(true);
			return;
		}

		s_SceneCtx.Progress.store(0.9f);
		// Store Ref; transferred via FinalizeSceneLoad on main thread.
		s_SceneCtx.LoadedScene = scene;
		s_SceneCtx.Progress.store(1.0f);
		s_SceneCtx.IsDone.store(true);
	}

	void LoadingSystem::LoadSceneAsync(const std::string& path)
	{
		s_SceneCtx.TargetPath = path;
		s_SceneCtx.Progress.store(0.0f);
		s_SceneCtx.IsDone.store(false);
		s_SceneCtx.HasError.store(false);
		s_SceneCtx.ErrorMessage.clear();
		s_SceneCtx.LoadedScene.reset();

		s_SceneThread = std::thread(&LoadingSystem::SceneWorker, path);
	}

	bool  LoadingSystem::IsSceneLoadDone()    { return s_SceneCtx.IsDone.load(); }
	float LoadingSystem::GetSceneLoadProgress() { return s_SceneCtx.Progress.load(); }

	Ref<Scene> LoadingSystem::FinalizeSceneLoad()
	{
		ASSERT(s_SceneCtx.IsDone.load(), "FinalizeSceneLoad called before scene worker finished");
		if (s_SceneThread.joinable())
			s_SceneThread.join();
		Ref<Scene> result = std::move(s_SceneCtx.LoadedScene);
		s_SceneCtx.LoadedScene = nullptr;
		return result;
	}

	// -----------------------------------------------------------------------
	// Generic queries
	// -----------------------------------------------------------------------
	float LoadingSystem::GetProgress() { return s_StartupDone.load()
		? s_SceneCtx.Progress.load()
		: s_StartupProgress.load(); }

	bool  LoadingSystem::IsLoading()   { return !s_StartupDone.load() || !s_SceneCtx.IsDone.load(); }

	void  LoadingSystem::Reset()
	{
		s_SceneCtx.Progress.store(0.0f);
		s_SceneCtx.IsDone.store(true); // treat as "not loading"
		s_SceneCtx.HasError.store(false);
		s_SceneCtx.ErrorMessage.clear();
	}
}

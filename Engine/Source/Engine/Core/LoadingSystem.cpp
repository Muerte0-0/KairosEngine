#include "kepch.h"
#include "LoadingSystem.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneSerializer.h"
#include "Engine/Assets/AssetRegistry.h"
#include "Engine/Debugging/Log.h"
#include "Engine/Core/Application.h"
#include "Engine/Assets/AssetManager.h"
#include "Engine/Scene/Entity.h"
#include "Engine/Scene/Components.h"
#include "Engine/Utils/PrimitiveMeshFactory.h"

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
	std::mutex          LoadingSystem::s_StatusMutex;
	Ref<Scene>          LoadingSystem::s_StartupScene = nullptr;
	std::vector<AssetHandle> LoadingSystem::s_StartupMeshQueue;
	std::vector<std::string> LoadingSystem::s_StartupPrimitiveQueue;
	size_t             LoadingSystem::s_StartupMeshIndex = 0;
	size_t             LoadingSystem::s_StartupPrimIndex = 0;

	Ref<Scene>          LoadingSystem::s_SceneResolveScene = nullptr;
	std::vector<AssetHandle> LoadingSystem::s_SceneResolveMeshQueue;
	std::vector<std::string> LoadingSystem::s_SceneResolvePrimitiveQueue;
	size_t             LoadingSystem::s_SceneResolveMeshIndex = 0;
	size_t             LoadingSystem::s_SceneResolvePrimIndex = 0;
	std::atomic<float> LoadingSystem::s_SceneResolveProgress{ 0.0f };

	thread_local SceneLoadContext* LoadingSystem::t_EnqueueSceneCtx = nullptr;

	// -----------------------------------------------------------------------
	// Internal helpers
	// -----------------------------------------------------------------------
	void LoadingSystem::SetStatus(const std::string& text)
	{
		std::lock_guard lock(s_StatusMutex);
		s_StatusText = text;
		if (s_StatusCallback)
			s_StatusCallback(text);
	}

	// -----------------------------------------------------------------------
	// Startup loading
	// -----------------------------------------------------------------------
	void LoadingSystem::StartupWorker()
	{
		// Startup worker does CPU-only work that can run before editor startup.
		// IMPORTANT: No Vulkan calls here. GPU work happens in FinalizeStartup().

		// CPU-only (spawns external process if compilation is needed).
		SetStatus("Checking shaders");
		Application::Get().EnsureApplicationShadersCompiled();
		s_StartupProgress.store(0.20f);

		// Keep remaining progress budget for main-thread editor initialization
		// (project selection, asset scan, scene deserialize, layer OnAttach).
		SetStatus("Waiting for editor initialization");
		
		// Cap worker at 0.60; main thread owns 0.60 -> 1.00.
		s_StartupProgress.store(0.60f);
		s_StartupDone.store(true);
	}

	void LoadingSystem::StartStartupLoading(std::function<void(const std::string&)> statusCallback)
	{
		{
			std::lock_guard lock(s_StatusMutex);
			s_StatusCallback = std::move(statusCallback);
		}
		s_StartupProgress.store(0.0f);
		s_StartupDone.store(false);
		SetStatus("Initializing Engine");

		// Reset main-thread startup queues
		s_StartupScene = nullptr;
		s_StartupMeshQueue.clear();
		s_StartupPrimitiveQueue.clear();
		s_StartupMeshIndex = 0;
		s_StartupPrimIndex = 0;

		s_StartupThread = std::thread(&LoadingSystem::StartupWorker);
	}

	bool  LoadingSystem::IsStartupDone()       { return s_StartupDone.load(); }
	float LoadingSystem::GetStartupProgress()  { return s_StartupProgress.load(); }
	std::string LoadingSystem::GetCurrentStatusText()
	{
		std::lock_guard lock(s_StatusMutex);
		return s_StatusText;
	}

	void LoadingSystem::SetStartupProgressMainThread(float progress)
	{
		s_StartupProgress.store(progress);
	}

	void LoadingSystem::SetStatusTextMainThread(const std::string& text)
	{
		SetStatus(text);
	}

	void LoadingSystem::SetStartupScene(const Ref<Scene>& scene)
	{
		s_StartupScene = scene;
	}

	void LoadingSystem::EnqueueStartupMeshAsset(AssetHandle handle)
	{
		EnqueueMesh(handle);
	}

	void LoadingSystem::EnqueueStartupPrimitive(const std::string& key)
	{
		EnqueuePrimitive(key);
	}

	void LoadingSystem::EnqueueMesh(AssetHandle handle)
	{
		if (static_cast<uint64_t>(handle) == NullAssetHandle)
			return;

		if (t_EnqueueSceneCtx)
			t_EnqueueSceneCtx->MeshAssets.push_back(handle);
		else
			s_StartupMeshQueue.push_back(handle);
	}

	void LoadingSystem::EnqueuePrimitive(const std::string& key)
	{
		if (key.empty())
			return;

		if (t_EnqueueSceneCtx)
			t_EnqueueSceneCtx->Primitives.push_back(key);
		else
			s_StartupPrimitiveQueue.push_back(key);
	}

	static float StartupResolveProgress(size_t done, size_t total)
	{
		if (total == 0) return 1.0f;
		return static_cast<float>(done) / static_cast<float>(total);
	}

	bool LoadingSystem::PumpStartupMainThreadWork(uint32_t meshBudgetPerFrame, uint32_t primBudgetPerFrame)
	{
		// No scene assigned yet -> nothing to do.
		if (!s_StartupScene)
			return true;

		// Resolve mesh assets
		uint32_t meshBudget = meshBudgetPerFrame;
		while (meshBudget > 0 && s_StartupMeshIndex < s_StartupMeshQueue.size())
		{
			const AssetHandle handle = s_StartupMeshQueue[s_StartupMeshIndex++];
			--meshBudget;

			SetStatusTextMainThread(
				"Loading mesh assets (" + std::to_string(s_StartupMeshIndex) + "/" + std::to_string(s_StartupMeshQueue.size()) + ")");

			Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(handle); // may import + upload (main thread only)
			if (!mesh)
				continue;

			// Assign to all entities that reference this handle and are not yet resolved.
			s_StartupScene->EachEntity([&](Entity e)
			{
				if (!e.HasComponent<MeshComponent>())
					return;
				auto& mc = e.GetComponent<MeshComponent>();
				if (mc.MeshAssetHandle == handle && !mc.HasMesh())
					mc.SetMeshAsset(handle, mesh, mesh->GetMaterials());
			});
		}

		// Resolve primitive meshes (can upload to GPU via Mesh::Create)
		uint32_t primBudget = primBudgetPerFrame;
		while (primBudget > 0 && s_StartupPrimIndex < s_StartupPrimitiveQueue.size())
		{
			const std::string key = s_StartupPrimitiveQueue[s_StartupPrimIndex++];
			--primBudget;

			SetStatusTextMainThread(
				"Building primitives (" + std::to_string(s_StartupPrimIndex) + "/" + std::to_string(s_StartupPrimitiveQueue.size()) + ")");

			Ref<Mesh> mesh = PrimitiveMeshFactory::GetOrCreate(key);
			if (!mesh)
				continue;

			s_StartupScene->EachEntity([&](Entity e)
			{
				if (!e.HasComponent<MeshComponent>())
					return;
				auto& mc = e.GetComponent<MeshComponent>();
				if (mc.IsPrimitive() && mc.PrimitiveKey == key && !mc.HasMesh())
					mc.SetPrimitiveMesh(key, mesh);
			});
		}

		const size_t total = s_StartupMeshQueue.size() + s_StartupPrimitiveQueue.size();
		const size_t done  = s_StartupMeshIndex + s_StartupPrimIndex;
		const float t = StartupResolveProgress(done, total);
		SetStartupProgressMainThread(0.85f + 0.14f * t); // reserve last 1% for "Ready"

		const bool doneAll = (s_StartupMeshIndex >= s_StartupMeshQueue.size())
			&& (s_StartupPrimIndex >= s_StartupPrimitiveQueue.size());
		if (doneAll)
		{
			SetStatusTextMainThread("Finalizing...");
			// Release startup references/queues so resources can be freed normally
			// (avoid holding Mesh buffers alive beyond Renderer shutdown).
			s_StartupScene = nullptr;
			s_StartupMeshQueue.clear();
			s_StartupPrimitiveQueue.clear();
		}
		return doneAll;
	}

	void LoadingSystem::FinalizeStartup()
	{
		ASSERT(s_StartupDone.load(), "FinalizeStartup called before startup worker finished")
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

		s_SceneCtx.MeshAssets.clear();
		s_SceneCtx.Primitives.clear();
		t_EnqueueSceneCtx = &s_SceneCtx;
		s_SceneCtx.Progress.store(0.4f);

		const bool ok = serializer.Deserialize(path);
		t_EnqueueSceneCtx = nullptr;
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
		s_SceneCtx.MeshAssets.clear();
		s_SceneCtx.Primitives.clear();

		s_SceneThread = std::thread(&LoadingSystem::SceneWorker, path);
	}

	bool  LoadingSystem::IsSceneLoadDone()    { return s_SceneCtx.IsDone.load(); }
	float LoadingSystem::GetSceneLoadProgress()
	{
		// While resolving assets on main thread, expose combined progress to loading UI.
		if (s_SceneResolveScene)
			return s_SceneResolveProgress.load();
		return s_SceneCtx.Progress.load();
	}

	void LoadingSystem::StartSceneTransitionResolve(const Ref<Scene>& scene)
	{
		s_SceneResolveScene = scene;
		s_SceneResolveMeshQueue = s_SceneCtx.MeshAssets;
		s_SceneResolvePrimitiveQueue = s_SceneCtx.Primitives;
		s_SceneResolveMeshIndex = 0;
		s_SceneResolvePrimIndex = 0;
		s_SceneResolveProgress.store(0.0f);
	}

	bool LoadingSystem::PumpSceneTransitionMainThreadWork(uint32_t meshBudgetPerFrame, uint32_t primBudgetPerFrame)
	{
		if (!s_SceneResolveScene)
			return true;

		uint32_t meshBudget = meshBudgetPerFrame;
		while (meshBudget > 0 && s_SceneResolveMeshIndex < s_SceneResolveMeshQueue.size())
		{
			const AssetHandle handle = s_SceneResolveMeshQueue[s_SceneResolveMeshIndex++];
			--meshBudget;

			SetStatusTextMainThread(
				"Loading mesh assets (" + std::to_string(s_SceneResolveMeshIndex) + "/" + std::to_string(s_SceneResolveMeshQueue.size()) + ")");

			Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(handle);
			if (!mesh)
				continue;

			s_SceneResolveScene->EachEntity([&](Entity e)
			{
				if (!e.HasComponent<MeshComponent>())
					return;
				auto& mc = e.GetComponent<MeshComponent>();
				if (mc.MeshAssetHandle == handle && !mc.HasMesh())
					mc.SetMeshAsset(handle, mesh, mesh->GetMaterials());
			});
		}

		uint32_t primBudget = primBudgetPerFrame;
		while (primBudget > 0 && s_SceneResolvePrimIndex < s_SceneResolvePrimitiveQueue.size())
		{
			const std::string key = s_SceneResolvePrimitiveQueue[s_SceneResolvePrimIndex++];
			--primBudget;

			SetStatusTextMainThread(
				"Building primitives (" + std::to_string(s_SceneResolvePrimIndex) + "/" + std::to_string(s_SceneResolvePrimitiveQueue.size()) + ")");

			Ref<Mesh> mesh = PrimitiveMeshFactory::GetOrCreate(key);
			if (!mesh)
				continue;

			s_SceneResolveScene->EachEntity([&](Entity e)
			{
				if (!e.HasComponent<MeshComponent>())
					return;
				auto& mc = e.GetComponent<MeshComponent>();
				if (mc.IsPrimitive() && mc.PrimitiveKey == key && !mc.HasMesh())
					mc.SetPrimitiveMesh(key, mesh);
			});
		}

		const size_t total = s_SceneResolveMeshQueue.size() + s_SceneResolvePrimitiveQueue.size();
		const size_t done  = s_SceneResolveMeshIndex + s_SceneResolvePrimIndex;
		const float t = StartupResolveProgress(done, total);

		// Scene transition progress: 0.0 -> 0.85 = async deserialize, 0.85 -> 1.0 = resolve.
		s_SceneResolveProgress.store(0.85f + 0.15f * t);

		const bool doneAll = (s_SceneResolveMeshIndex >= s_SceneResolveMeshQueue.size())
			&& (s_SceneResolvePrimIndex >= s_SceneResolvePrimitiveQueue.size());
		if (doneAll)
		{
			SetStatusTextMainThread("Finalizing...");
			s_SceneResolveScene = nullptr;
			s_SceneResolveMeshQueue.clear();
			s_SceneResolvePrimitiveQueue.clear();
		}
		return doneAll;
	}

	Ref<Scene> LoadingSystem::FinalizeSceneLoad()
	{
		ASSERT(s_SceneCtx.IsDone.load(), "FinalizeSceneLoad called before scene worker finished")
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

		s_StartupScene = nullptr;
		s_StartupMeshQueue.clear();
		s_StartupPrimitiveQueue.clear();
		s_StartupMeshIndex = 0;
		s_StartupPrimIndex = 0;

		s_SceneResolveScene = nullptr;
		s_SceneResolveMeshQueue.clear();
		s_SceneResolvePrimitiveQueue.clear();
		s_SceneResolveMeshIndex = 0;
		s_SceneResolvePrimIndex = 0;
		s_SceneResolveProgress.store(0.0f);
	}
}

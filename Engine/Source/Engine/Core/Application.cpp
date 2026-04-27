#include "kepch.h"
#include "Application.h"
#include "Engine/Core/LoadingSystem.h"
#include "Engine/Utils/PlatformUtils.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Utils/PrimitiveMeshFactory.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Engine/Project/Project.h"

constexpr float MIN_DELTA_TIME  = 0.001f;
constexpr float MAX_DELTA_TIME  = 0.1f;
constexpr float FIXED_FRAME_TIME = 1.0f / 60.0f;

extern bool g_ApplicationRunning;

namespace Engine
{
	namespace
	{
		namespace fs = std::filesystem;
		
		struct ProcessResult
		{
			int         ExitCode = -1;
			std::string Output;
		};

		std::string QuoteArgument(const std::string& value)
		{
			std::string quoted = "\"";
			for (const char character : value)
			{
				if (character == '"')
					quoted += "\\\"";
				else
					quoted += character;
			}
			quoted += "\"";
			return quoted;
		}

		std::string JoinCommand(const std::vector<std::string>& arguments)
		{
			std::ostringstream builder;
			for (size_t index = 0; index < arguments.size(); ++index)
			{
				if (index > 0)
					builder << ' ';
				builder << QuoteArgument(arguments[index]);
			}
			return builder.str();
		}

		ProcessResult RunProcess(const std::vector<std::string>& arguments)
		{
			ProcessResult result;

#ifdef PLATFORM_WINDOWS
			const std::string innerCommand = JoinCommand(arguments) + " 2>&1";
			const std::string command = "cmd /d /s /c \"" + innerCommand + "\"";
			FILE* pipe = _popen(command.c_str(), "r");
#else
			const std::string command = JoinCommand(arguments) + " 2>&1";
			FILE* pipe = popen(command.c_str(), "r");
#endif
			if (!pipe)
			{
				result.Output = "Failed to start process.";
				return result;
			}

			std::array<char, 512> buffer{};
			while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr)
				result.Output += buffer.data();

#if defined(PLATFORM_WINDOWS)
			result.ExitCode = _pclose(pipe);
#else
			result.ExitCode = pclose(pipe);
#endif
			return result;
		}

		// ----------------------------------------------------------------
		// Shader compiler resolution
		// ----------------------------------------------------------------

		fs::path GetShaderCompilerBinaryName()
		{
#ifdef PLATFORM_WINDOWS
			return "ShaderCompiler.exe";
#else
			return "ShaderCompiler";
#endif
		}

		std::optional<fs::path> ResolveShaderCompilerPath(const fs::path& workspaceRoot)
		{
			// 1. Look for the compiler next to the running executable first.
			if (const auto executablePath = PlatformUtils::GetExecutablePath())
			{
				const fs::path siblingCandidate =
					executablePath->parent_path().parent_path()
					/ "Tools" / "ShaderCompiler"
					/ GetShaderCompilerBinaryName();

				if (fs::exists(siblingCandidate))
					return fs::weakly_canonical(siblingCandidate);
			}

			// 2. Fall back to scanning the workspace Binaries tree.
			const fs::path binariesRoot = workspaceRoot / "Binaries";
			if (!fs::exists(binariesRoot))
				return std::nullopt;

			for (const auto& entry : fs::directory_iterator(binariesRoot))
			{
				if (!entry.is_directory())
					continue;

				const fs::path candidate =
					entry.path() / "Tools" / "ShaderCompiler"
					/ GetShaderCompilerBinaryName();

				if (fs::exists(candidate))
					return fs::weakly_canonical(candidate);
			}

			return std::nullopt;
		}

		bool PrintCommandOutput(const ProcessResult& result)
		{
			if (!result.Output.empty())
			{
				std::cout << result.Output;
				if (!result.Output.ends_with('\n'))
					std::cout << '\n';
			}
			return result.ExitCode == 0;
		}
	}

	// ====================================================================
	// Application
	// ====================================================================

	static Application* s_Application = nullptr;

	static void GLFWErrorCallback(int error, const char* description)
	{
		std::cerr << "[GLFW Error]: " << description << '\n';
	}

	Application::Application()
	{
		s_Application = this;
	}

	Application::~Application()
	{
		s_Application = nullptr;
	}

	void Application::Initialize()
	{		
		glfwSetErrorCallback(GLFWErrorCallback);
		glfwInit();

		WindowSpecification spec = GetApplicationSpecs().WindowSpec;
		spec.EventCallback = [this](Event& event) { RaiseEvent(event); };

		m_Window = CreateRef<Window>(spec);
		m_Window->Create();

		// Resolve shader directory relative to the workspace root when the
		// configured path is relative.
		fs::path shaderDirectory = GetApplicationSpecs().ShaderSourcePath;
		if (const auto workspaceRoot = PlatformUtils::ResolveWorkspaceRoot())
		{
			if (shaderDirectory.is_relative())
				shaderDirectory = *workspaceRoot / shaderDirectory;
		}
		shaderDirectory /= "Compiled";

		Renderer::Init(API::Vulkan, m_Window->GetHandle(), shaderDirectory);
		
		m_ImGuiLayer = ImGuiLayer::Create();
		m_ImGuiLayer->OnAttach();
		
		// Kick async startup loading — engine starts in Loading state.
		m_EngineState  = EngineState::Loading;
		m_LoadingPhase = LoadingPhase::EditorStartup;
		m_LoadingScreen.OnLoadingStarted();
		LoadingSystem::StartStartupLoading();
	}

	void Application::Shutdown()
	{
		m_ImGuiLayer->OnDetach();

		for (auto& layer : views::reverse(m_LayerStack))
			layer->OnDetach();

		// Destroy layer objects now — they hold scene/mesh/material Vulkan resources.
		// Must happen before Renderer::Shutdown() destroys the device.
		m_LayerStack.clear();
		m_ImGuiLayer.reset();

		// Drop all loaded assets (Mesh VkBuffers held by EditorAssetManager).
		Project::SetActive(nullptr);

		// Release primitive mesh cache before device teardown.
		PrimitiveMeshFactory::Shutdown();

		// Drop default material + backend fallback textures, then destroy device.
		Renderer::Shutdown();

		m_Window->Destroy();
		glfwTerminate();
	}

	void Application::RequestSceneChange(const std::string& path)
	{
		m_EngineState  = EngineState::Loading;
		m_LoadingPhase = LoadingPhase::SceneTransition;
		m_LoadingScreen.OnLoadingStarted();
		m_PendingSceneTransition = nullptr;
		m_SceneTransitionResolveStarted = false;
		m_SceneTransitionPrimed = false;
		LoadingSystem::LoadSceneAsync(path);
	}

	void Application::TickLoadingState(float deltaTime)
	{
		m_LoadingScreen.Update(deltaTime);

		if (m_LoadingPhase == LoadingPhase::EditorStartup)
		{
			if (LoadingSystem::IsStartupDone())
			{
				// NOTE: Some layers perform heavy CPU work in OnAttach (project open,
				// asset scan, scene deserialize). Defer that work until after first
				// frames so loading screen can present immediately.
				AttachLayersDuringLoading();

				if (m_LayersAttached)
				{
					// Incrementally resolve scene assets (imports + GPU uploads) on main thread
					// so loading UI keeps rendering.
					const bool assetsDone = LoadingSystem::PumpStartupMainThreadWork(1, 1);
					if (assetsDone)
					{
						LoadingSystem::SetStatusTextMainThread("Ready.");
						LoadingSystem::SetStartupProgressMainThread(1.0f);

						m_LoadingScreen.OnLoadingFinished();
						if (!m_LoadingScreen.IsFadingOut())
						{
							LoadingSystem::FinalizeStartup();
							m_EngineState = EngineState::Running;
						}
					}
				}
			}
		}
		else if (m_LoadingPhase == LoadingPhase::SceneTransition)
		{
			if (!m_SceneTransitionResolveStarted && LoadingSystem::IsSceneLoadDone())
			{
				// Join worker + capture scene. Asset resolve happens incrementally on main thread.
				m_PendingSceneTransition = LoadingSystem::FinalizeSceneLoad();
				LoadingSystem::StartSceneTransitionResolve(m_PendingSceneTransition);
				m_SceneTransitionResolveStarted = true;

				LoadingSystem::SetStatusTextMainThread("Resolving scene assets...");
				if (!m_SceneTransitionPrimed)
				{
					m_SceneTransitionPrimed = true;
					return; // let UI render status once before first blocking import
				}
			}

			if (m_SceneTransitionResolveStarted)
			{
				const bool done = LoadingSystem::PumpSceneTransitionMainThreadWork(1, 1);
				if (done)
				{
					m_LoadingScreen.OnLoadingFinished();
					if (!m_LoadingScreen.IsFadingOut())
					{
						for (auto& layer : m_LayerStack)
							layer->OnSceneLoaded(m_PendingSceneTransition);
						m_PendingSceneTransition = nullptr;
						m_EngineState = EngineState::Running;
					}
				}
			}
		}
	}

	void Application::AttachLayersDuringLoading()
	{
		if (m_LayersAttached)
			return;

		const size_t total = m_LayerStack.size();
		if (total == 0)
		{
			m_LayersAttached = true;
			return;
		}

		// Two-phase attach:
		// - frame N: publish status/progress so UI can render it
		// - frame N+1: run potentially blocking OnAttach()
		//
		// This avoids the "stuck at old status then jump" feel when OnAttach
		// does heavy synchronous work (project open, asset scan, scene load).
		const size_t index = m_NextLayerToAttach;
		if (index < total)
		{
			const float t = static_cast<float>(index) / static_cast<float>(total);
			LoadingSystem::SetStatusTextMainThread(
				"Initializing editor (" + std::to_string(index) + "/" + std::to_string(total) + ")...");
			LoadingSystem::SetStartupProgressMainThread(0.95f + 0.05f * t);

			if (!m_LayerAttachPrimed)
			{
				m_LayerAttachPrimed = true;
				return;
			}

			m_LayerAttachPrimed = false;
			m_LayerStack[index]->OnAttach();
			++m_NextLayerToAttach;
		}

		if (m_NextLayerToAttach >= total)
		{
			LoadingSystem::SetStatusTextMainThread("Ready.");
			LoadingSystem::SetStartupProgressMainThread(1.0f);
			m_LayersAttached = true;
		}
	}

	void Application::RenderLoadingScreen()
	{
		const float progress   = (m_LoadingPhase == LoadingPhase::EditorStartup)
			? LoadingSystem::GetStartupProgress()
			: LoadingSystem::GetSceneLoadProgress();

		const std::string status = (m_LoadingPhase == LoadingPhase::EditorStartup)
			? LoadingSystem::GetCurrentStatusText()
			: std::string{};

		m_LoadingScreen.Render(m_LoadingPhase, progress, status);
	}

	void Application::Run()
	{
		g_ApplicationRunning = true;

		float lastTime    = GetTime();
		float accumulator = 0.0f;

		while (g_ApplicationRunning)
		{
			glfwPollEvents();

			if (m_Window->ShouldClose())
			{
				Stop();
				break;
			}

			const float currentTime = static_cast<float>(glfwGetTime());
			const float deltaTime   = glm::clamp(currentTime - lastTime, MIN_DELTA_TIME, MAX_DELTA_TIME);
			lastTime = currentTime;

			accumulator += deltaTime;

			// Fixed update
			while (accumulator >= FIXED_FRAME_TIME)
			{
				if (!m_IsMinimized)
				{
					m_ImGuiLayer->OnFixedUpdate(FIXED_FRAME_TIME);
					if (m_EngineState == EngineState::Running)
					{
						for (const auto& layer : m_LayerStack)
							layer->OnFixedUpdate(FIXED_FRAME_TIME);
					}
				}
				accumulator -= FIXED_FRAME_TIME;
			}

			// Per-frame update
			if (!m_IsMinimized)
			{
				m_ImGuiLayer->OnUpdate(deltaTime);

				if (m_EngineState == EngineState::Running)
				{
					for (const auto& layer : m_LayerStack)
						layer->OnUpdate(deltaTime);
				}
				else
				{
					TickLoadingState(deltaTime);
				}

				Renderer::BeginScene();

				if (m_EngineState == EngineState::Running)
				{
					for (const auto& layer : m_LayerStack)
						layer->OnRender();
				}
				m_ImGuiLayer->OnRender();

				m_ImGuiLayer->Begin(deltaTime);

				Renderer::DrawFrame();

				if (m_EngineState == EngineState::Running)
				{
					m_ImGuiLayer->OnImGuiRender();
					for (const auto& layer : m_LayerStack)
						layer->OnImGuiRender();
				}
				else
				{
					// Render loading screen on top; suppress editor UI.
					RenderLoadingScreen();
				}

				m_ImGuiLayer->End();
				Renderer::EndScene();

				m_Window->Update();
			}

			const float sleepTime = FIXED_FRAME_TIME - (GetTime() - currentTime);
			if (sleepTime > 0)
				std::this_thread::sleep_for(std::chrono::duration<float>(sleepTime));
		}
	}

	void Application::Stop()
	{
		g_ApplicationRunning = false;
	}

	void Application::RaiseEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(&Application::OnWindowResize));
		dispatcher.Dispatch<WindowClosedEvent>(BIND_EVENT_FN(&Application::OnWindowClosed));

		m_ImGuiLayer->OnEvent(event);
		if (event.Handled)
			return;

		// Suppress all layer input while loading.
		if (m_EngineState == EngineState::Loading)
			return;

		for (auto& layer : views::reverse(m_LayerStack))
		{
			layer->OnEvent(event);
			if (event.Handled)
				break;
		}
	}

	glm::vec2 Application::GetFramebufferSize() const
	{
		return m_Window->GetFramebufferSize();
	}

	Application& Application::Get()
	{
		assert(s_Application);
		return *s_Application;
	}

	float Application::GetTime()
	{
		return static_cast<float>(glfwGetTime());
	}

	void Application::EnsureApplicationShadersCompiled() const
	{
		const ApplicationSpecification appSpec = GetApplicationSpecs();

		if (!appSpec.CompileShadersOnStartup)
			return;

		const auto workspaceRoot = PlatformUtils::ResolveWorkspaceRoot();
		if (!workspaceRoot)
		{
			LOG(LogLevel::Error, "Failed to resolve KairosEngine workspace root.");
			exit(EXIT_FAILURE);
		}

		fs::path shaderSourcePath = appSpec.ShaderSourcePath;
		if (shaderSourcePath.empty())
		{
			LOG(LogLevel::Error, "Shader compilation is enabled but no shader source path was provided.");
			exit(EXIT_FAILURE);
		}

		if (shaderSourcePath.is_relative())
			shaderSourcePath = *workspaceRoot / shaderSourcePath;

		if (!fs::exists(shaderSourcePath))
		{
			LOG(LogLevel::Error, "Shader source path does not exist: {}", shaderSourcePath.string());
			exit(EXIT_FAILURE);
		}

		const auto shaderCompilerPath = ResolveShaderCompilerPath(*workspaceRoot);
		if (!shaderCompilerPath)
		{
			LOG(LogLevel::Error, "Failed to resolve the ShaderCompiler binary.");
			exit(EXIT_FAILURE);
		}

		// Check whether any shaders need recompilation.
		const ProcessResult checkResult = RunProcess({
			shaderCompilerPath->string(),
			"--source", shaderSourcePath.string(),
			"--check"
		});
		PrintCommandOutput(checkResult);

		if (checkResult.ExitCode == 0)
			return; // All up-to-date.

		if (checkResult.ExitCode != 2)
		{
			LOG(LogLevel::Error, "ShaderCompiler check failed.");
			exit(EXIT_FAILURE);
		}

		// Compile stale / missing shaders.
		const ProcessResult compileResult = RunProcess({
			shaderCompilerPath->string(),
			"--source", shaderSourcePath.string()
		});

		if (!PrintCommandOutput(compileResult))
		{
			LOG(LogLevel::Error, "ShaderCompiler failed to compile application shaders.");
			std::exit(EXIT_FAILURE);
		}
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_IsMinimized = true;
			return true;
		}

		Renderer::WindowResized();
		m_IsMinimized = false;
		return false;
	}

	bool Application::OnWindowClosed(WindowClosedEvent& e)
	{
		Stop();
		return true;
	}
}

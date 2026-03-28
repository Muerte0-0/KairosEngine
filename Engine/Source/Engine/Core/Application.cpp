#include "kepch.h"
#include "Application.h"
#include "Engine/Renderer/Renderer.h"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#else
#include <limits.h>
#include <unistd.h>
#endif

constexpr float MIN_DELTA_TIME = 0.001f;
constexpr float MAX_DELTA_TIME = 0.1f;
constexpr float FIXED_FRAME_TIME = 1.0f / 60.0f; // Target 60 FPS for Fixed Update

extern bool g_ApplicationRunning;

namespace Engine
{
	namespace
	{
		namespace fs = std::filesystem;

		struct ProcessResult
		{
			int ExitCode = -1;
			std::string Output;
		};

		std::string QuoteArgument(const std::string& value)
		{
			std::string quoted = "\"";
			for (const char character : value)
			{
				if (character == '"')
				{
					quoted += "\\\"";
				}
				else
				{
					quoted += character;
				}
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
				{
					builder << ' ';
				}

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

			std::array<char, 512> buffer {};
			while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr)
			{
				result.Output += buffer.data();
			}

#ifdef PLATFORM_WINDOWS
			result.ExitCode = _pclose(pipe);
#else
			result.ExitCode = pclose(pipe);
#endif
			return result;
		}

		std::optional<fs::path> GetExecutablePath()
		{
#ifdef PLATFORM_WINDOWS
			std::wstring buffer(MAX_PATH, L'\0');
			DWORD length = 0;

			while (true)
			{
				length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
				if (length == 0)
				{
					return std::nullopt;
				}

				if (length < buffer.size())
				{
					buffer.resize(length);
					return fs::path(buffer);
				}

				buffer.resize(buffer.size() * 2);
			}
#else
			std::vector<char> buffer(PATH_MAX, '\0');

			while (true)
			{
				const ssize_t length = readlink("/proc/self/exe", buffer.data(), buffer.size());
				if (length < 0)
				{
					return std::nullopt;
				}

				if (static_cast<size_t>(length) < buffer.size())
				{
					return fs::path(std::string(buffer.data(), static_cast<size_t>(length)));
				}

				buffer.resize(buffer.size() * 2);
			}
#endif
		}

		std::optional<fs::path> FindWorkspaceRoot(fs::path startPath)
		{
			if (startPath.empty())
			{
				return std::nullopt;
			}

			if (fs::is_regular_file(startPath))
			{
				startPath = startPath.parent_path();
			}

			for (fs::path current = fs::weakly_canonical(startPath); !current.empty(); current = current.parent_path())
			{
				if (fs::exists(current / "KairosEngine-Setup.lua"))
				{
					return current;
				}

				if (current == current.root_path())
				{
					break;
				}
			}

			return std::nullopt;
		}

		std::optional<fs::path> ResolveWorkspaceRoot()
		{
			if (const auto executablePath = GetExecutablePath())
			{
				if (const auto root = FindWorkspaceRoot(*executablePath))
				{
					return root;
				}
			}

			return FindWorkspaceRoot(fs::current_path());
		}

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
			const auto executablePath = GetExecutablePath();
			if (executablePath)
			{
				const fs::path executableDir = executablePath->parent_path();
				const fs::path siblingToolsCompiler = executableDir.parent_path() / "Tools" / "ShaderCompiler" / GetShaderCompilerBinaryName();
				if (fs::exists(siblingToolsCompiler))
				{
					return fs::weakly_canonical(siblingToolsCompiler);
				}
			}

			const fs::path binariesRoot = workspaceRoot / "Binaries";
			if (!fs::exists(binariesRoot))
			{
				return std::nullopt;
			}

			for (const auto& entry : fs::directory_iterator(binariesRoot))
			{
				if (!entry.is_directory())
				{
					continue;
				}

				const fs::path candidate = entry.path() / "Tools" / "ShaderCompiler" / GetShaderCompilerBinaryName();
				if (fs::exists(candidate))
				{
					return fs::weakly_canonical(candidate);
				}
			}

			return std::nullopt;
		}

		bool PrintCommandOutput(const ProcessResult& result)
		{
			if (!result.Output.empty())
			{
				std::cout << result.Output;
				if (!result.Output.ends_with('\n'))
				{
					std::cout << '\n';
				}
			}

			return result.ExitCode == 0;
		}
	}

	static Application* s_Application = nullptr;
	
	static void GLFWErrorCallback(int error, const char* description)
	{
		cerr << "[GLFW Error]: " << description << '\n';
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
		EnsureApplicationShadersCompiled();

		glfwSetErrorCallback(GLFWErrorCallback);
		glfwInit();
		
		WindowSpecification spec = GetApplicationSpecs().WindowSpec;
		
		spec.EventCallback = [this](Event& event) {RaiseEvent(event); };
		
		m_Window = CreateRef<Window>(spec);
		m_Window->Create();

		fs::path shaderDirectory = GetApplicationSpecs().ShaderSourcePath;
		if (const optional<fs::path> workspaceRoot = ResolveWorkspaceRoot())
		{
			if (shaderDirectory.is_relative())
			{
				shaderDirectory = *workspaceRoot / shaderDirectory;
			}
		}
		shaderDirectory /= "Compiled";
		
		Renderer::Init(API::Vulkan, m_Window->GetHandle(), shaderDirectory);
		
		for (auto& layer : m_LayerStack)
			layer->OnAttach();
		
		m_ImGuiLayer = ImGuiLayer::Create();
		m_ImGuiLayer->OnAttach();
	}

	void Application::Shutdown()
	{
		m_ImGuiLayer->OnDetach();
		
		for (auto& layer : views::reverse(m_LayerStack))
			layer->OnDetach();
		
		m_Window->Destroy();
		glfwTerminate();
	}

	void Application::Run()
	{
		g_ApplicationRunning = true;

		float lastTime = GetTime();
		float accumulator = 0.0f;
		
		while (g_ApplicationRunning)
		{
			glfwPollEvents();
			
			if (m_Window->ShouldClose())
			{
				Stop();
				break;
			}
			
			float currentTime = static_cast<float>(glfwGetTime());
			float deltaTime = glm::clamp(currentTime - lastTime, MIN_DELTA_TIME, MAX_DELTA_TIME);
			lastTime = currentTime;
			
			accumulator += deltaTime;
            
			// Fixed Update
			while (accumulator >= FIXED_FRAME_TIME)
			{
				if (!m_IsMinimized)
				{
					m_ImGuiLayer->OnFixedUpdate(FIXED_FRAME_TIME);
					
					for (const auto& layer : m_LayerStack)
						layer->OnFixedUpdate(FIXED_FRAME_TIME);
				}
				accumulator -= FIXED_FRAME_TIME;
			}
			
			// Tick Update
			if (!m_IsMinimized)
			{
				m_ImGuiLayer->OnUpdate(deltaTime);
				
				for (const unique_ptr<Layer>& layer : m_LayerStack)
					layer->OnUpdate(deltaTime);
			
				// To-Do: Do This on the Render Thread
				for (const unique_ptr<Layer>& layer : m_LayerStack)
					layer->OnRender();
				
				m_ImGuiLayer->OnRender();
				
				Renderer::BeginFrame();
				m_ImGuiLayer->Begin();
				
				Renderer::DrawFrame();
				
				m_ImGuiLayer->OnImGuiRender();
				
				for (const auto& layer : m_LayerStack)
					layer->OnImGuiRender();
				
				m_ImGuiLayer->End();
				Renderer::EndFrame();
				
				m_Window->Update();
			}
			
			float sleepTime = FIXED_FRAME_TIME - (GetTime() - currentTime);
			
			if (sleepTime > 0)
				this_thread::sleep_for(chrono::duration<float>(sleepTime));
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
		if (event.Handled) return;
		
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

		const optional<fs::path> workspaceRoot = ResolveWorkspaceRoot();
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
		{
			shaderSourcePath = *workspaceRoot / shaderSourcePath;
		}

		if (!fs::exists(shaderSourcePath))
		{
			LOG(LogLevel::Error, "Shader source path does not exist: {}", shaderSourcePath.string());
			exit(EXIT_FAILURE);
		}

		const std::optional<fs::path> shaderCompilerPath = ResolveShaderCompilerPath(*workspaceRoot);
		if (!shaderCompilerPath)
		{
			LOG(LogLevel::Error, "Failed to resolve the ShaderCompiler binary.");
			exit(EXIT_FAILURE);
		}

		const vector<string> checkArguments = {
			shaderCompilerPath->string(),
			"--source",
			shaderSourcePath.string(),
			"--check"
		};

		const ProcessResult checkResult = RunProcess(checkArguments);
		PrintCommandOutput(checkResult);

		if (checkResult.ExitCode == 0)
		{
			return;
		}

		if (checkResult.ExitCode != 2)
		{
			LOG(LogLevel::Error, "ShaderCompiler check failed.");
			exit(EXIT_FAILURE);
		}

		const std::vector<std::string> compileArguments = {
			shaderCompilerPath->string(),
			"--source",
			shaderSourcePath.string()
		};

		const ProcessResult compileResult = RunProcess(compileArguments);
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

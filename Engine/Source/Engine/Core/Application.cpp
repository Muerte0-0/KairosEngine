#include "kepch.h"
#include "Application.h"
#include "Engine/Renderer/Renderer.h"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

constexpr float MIN_DELTA_TIME = 0.001f;
constexpr float MAX_DELTA_TIME = 0.1f;
constexpr float FIXED_FRAME_TIME = 1.0f / 60.0f; // Target 60 FPS for Fixed Update

extern bool g_ApplicationRunning;

namespace Engine
{
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
		glfwSetErrorCallback(GLFWErrorCallback);
		glfwInit();
		
		GetApplicationSpecs().WindowSpec.EventCallback = [this](Event& event) {RaiseEvent(event); };
		
		m_Window = CreateRef<Window>(GetApplicationSpecs().WindowSpec);
		m_Window->Create();
		
		Renderer::Init(API::Vulkan, m_Window->GetHandle());
		
		for (auto& layer : m_LayerStack)
			layer->OnAttach();
	}

	void Application::Shutdown()
	{
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
					for (const auto& layer : m_LayerStack)
						layer->OnFixedUpdate(FIXED_FRAME_TIME);
				}
				accumulator -= FIXED_FRAME_TIME;
			}
			
			// Tick Update
			if (!m_IsMinimized)
			{
				for (const unique_ptr<Layer>& layer : m_LayerStack)
					layer->OnUpdate(deltaTime);
			
				// To-Do: Do This on the Render Thread
				for (const unique_ptr<Layer>& layer : m_LayerStack)
					layer->OnRender();
			
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

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_IsMinimized = true;
			return true;
		}

		m_IsMinimized = false;
        
		return false;
	}

	bool Application::OnWindowClosed(WindowClosedEvent& e)
	{
		Stop();
		return true;
	}
}

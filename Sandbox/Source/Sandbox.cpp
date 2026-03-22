// -------- Entry Point -------- //
#include "Engine/Core/EntryPoint.h"
// ----------------------------- //

#include "Engine.h"

class Sandbox : public Engine::Application
{
public:
	Sandbox()
	{
		//PushLayer<EditorLayer>();
	}
    
	Engine::ApplicationSpecification GetApplicationSpecs() override
	{
		Engine::ApplicationSpecification appSpec;
		appSpec.Name = "Sandbox";
        
		Engine::WindowSpecification windowSpec;
		windowSpec.Title = "Sandbox";
		windowSpec.Width = 1280;
		windowSpec.Height = 720;
		windowSpec.CustomTitleBar = false;
		appSpec.WindowSpec = windowSpec;
        
		return appSpec;
	}
};

Engine::Application* Engine::CreateApplication(int argc, char** argv)
{
	return new Sandbox();
}

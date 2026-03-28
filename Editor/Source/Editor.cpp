// -------- Entry Point -------- //
#include "Engine/Core/EntryPoint.h"
// ----------------------------- //

#include "Engine.h"

#include "EditorLayer.h"

class Editor : public Engine::Application
{
public:
	Editor()
	{
		PushLayer<EditorLayer>();
	}
    
	Engine::ApplicationSpecification GetApplicationSpecs() const override
	{
		Engine::ApplicationSpecification appSpec;
		appSpec.Name = "KairosEditor";
		appSpec.ShaderSourcePath = "Editor/Assets/Shaders";
		appSpec.CompileShadersOnStartup = true;
        
		Engine::WindowSpecification windowSpec;
		windowSpec.Title = "Kairos Editor";
		windowSpec.Width = 1280;
		windowSpec.Height = 720;
		windowSpec.CustomTitleBar = false;
		appSpec.WindowSpec = windowSpec;
        
		return appSpec;
	}
};

Engine::Application* Engine::CreateApplication(int argc, char** argv)
{
	return new Editor();
}

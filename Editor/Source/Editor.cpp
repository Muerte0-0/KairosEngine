#include "Engine.h"

// -------- Entry Point -------- //
#include "Engine/Core/EntryPoint.h"
// ----------------------------- //

using namespace Engine;

class Editor : public Application
{
public:
	Editor()
	{
		//PushLayer<EditorLayer>();
	}
    
	ApplicationSpecification GetApplicationSpecs() override
	{
		ApplicationSpecification appSpec;
		appSpec.Name = "Kairos Editor";
        
		WindowSpecification windowSpec;
		windowSpec.Title = "Kairos Editor";
		windowSpec.Width = 1280;
		windowSpec.Height = 720;
		windowSpec.CustomTitleBar = false;
		appSpec.WindowSpec = windowSpec;
        
		return appSpec;
	}
};

Application* Engine::CreateApplication(int argc, char** argv)
{
	return new Editor();
}
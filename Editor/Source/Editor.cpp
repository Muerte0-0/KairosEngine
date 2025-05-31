#include "Engine.h"

// -------- Entry Point -------- //
#include "Engine/Core/EntryPoint.h"
// ----------------------------- //

#include "EditorLayer.h"

namespace Kairos
{
	class KairosEditor : public Application
	{
	public:
		KairosEditor()
		{
			PushLayer(new EditorLayer());
		}

		~KairosEditor() {}
	};

	Kairos::WindowProps* Kairos::GetWindowProps()
	{
		return new WindowProps(
			"Kairos Editor",
			1920, 1080
		);
	}

	Kairos::Application* Kairos::CreateApplication()
	{
		return new KairosEditor();
	}
}
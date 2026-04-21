#pragma once
#include "Engine.h"
#include "imgui.h"

namespace Kairos
{
	class EditorWindow
	{
	public:
		virtual ~EditorWindow() = default;
		virtual void OnImGuiRender() = 0;
		virtual const std::string& GetTitle() const = 0;
		bool IsOpen() const { return m_Open; }

		// Set by EditorLayer before calling OnImGuiRender so subclasses
		// can dock into the outer dockspace without hard-coding the ID.
		void SetOuterDockID(ImGuiID id) { m_OuterDockID = id; }

	protected:
		std::string m_Title;
		bool        m_Open        = true;
		ImGuiID     m_OuterDockID = 0;
	};
}

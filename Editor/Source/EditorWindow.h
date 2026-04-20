#pragma once
#include "Engine.h"

namespace Kairos
{
	class EditorWindow
	{
	public:
		virtual ~EditorWindow() = default;
		virtual void OnImGuiRender() = 0;
		virtual const std::string& GetTitle() const = 0;
		bool IsOpen() const { return m_Open; }
	protected:
		std::string m_Title;
		bool        m_Open = true;
	};
}

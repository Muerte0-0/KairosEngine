#pragma once
#include "Engine.h"

namespace Kairos
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& context);
		
		void SetContext(const Ref<Scene>& context) { m_Context = context; }
		
		void OnImGuiRender();
	private:
		void DrawEntityNode(Entity entity);
		
		Ref<Scene>		m_Context;
		Entity			m_SelectionContext;
		
	};
}

#pragma once
#include "Engine.h"
#include "Panel.h"

namespace Kairos
{
	class SceneHierarchyPanel : public Panel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& context);

		void SetContext(const Ref<Scene>& context)
		{
			m_SelectionContext = {};
			m_Context = context;
		}

		void OnImGuiRender() override;

		Entity GetSelectedEntity() const { return m_SelectionContext; }
		void   SetSelectedEntity(Entity entity) { m_SelectionContext = entity; }

	private:
		void DrawEntityNode(Entity entity);

		Ref<Scene>  m_Context;
		Entity      m_SelectionContext;
	};
}

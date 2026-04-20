#pragma once
#include "Engine.h"
#include "Panel.h"

namespace Kairos
{
	class PropertiesPanel : public Panel
	{
	public:
		PropertiesPanel() = default;

		void SetSelectedEntity(Entity entity) { m_SelectionContext = entity; }

		void OnImGuiRender() override;

	private:
		void DrawComponents(Entity entity);

		Entity m_SelectionContext;
	};
}

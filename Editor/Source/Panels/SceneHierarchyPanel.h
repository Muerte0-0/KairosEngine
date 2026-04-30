#pragma once
#include "Engine.h"
#include "Panel.h"
#include <functional>
#include <filesystem>

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

		// Called when user picks "Save As Prefab" on an entity.
		// EditorLayer wires this to open a save dialog + call SavePrefab.
		std::function<void(Entity)> OnSaveAsPrefab;

	private:
		void DrawEntityNode(Entity entity);

		Ref<Scene>  m_Context;
		Entity      m_SelectionContext;

		// Inline rename state
		Engine::UUID m_RenameEntityUUID    = 0;
		bool         m_IsRenamingEntity    = false;
		bool         m_RenameEntityFocused = false;
		char         m_EntityRenameBuffer[256] = {};
	};
}

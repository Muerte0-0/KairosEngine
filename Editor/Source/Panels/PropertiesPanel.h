#pragma once
#include "Engine.h"
#include "Panel.h"

namespace Kairos
{
	class PropertiesPanel : public Panel
	{
	public:
		PropertiesPanel() = default;

		void SetSelectedEntity(Entity entity)
		{
			m_SelectionContext = entity;
			m_SelectedAsset    = Engine::AssetHandle(Engine::NullAssetHandle);
		}

		void SetSelectedAsset(Engine::AssetHandle handle)
		{
			m_SelectedAsset    = handle;
			m_SelectionContext = {};   // clear entity
		}

		void OnImGuiRender() override;

		// Called after any component edit (transform, add, remove, etc.).
		// EditorLayer wires this to mark the prefab dirty when in prefab edit mode.
		std::function<void()> OnEntityModified;
		std::function<void(Entity)> OnApplyPrefabInstance;
		std::function<void(Entity)> OnRevertPrefabInstance;

	private:
		void DrawComponents(Entity entity);
		void DrawAssetInspector(Engine::AssetHandle handle);

		Entity               m_SelectionContext;
		Engine::AssetHandle  m_SelectedAsset = Engine::AssetHandle(Engine::NullAssetHandle);
	};
}

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

	private:
		void DrawComponents(Entity entity);
		void DrawAssetInspector(Engine::AssetHandle handle);

		Entity               m_SelectionContext;
		Engine::AssetHandle  m_SelectedAsset = Engine::AssetHandle(Engine::NullAssetHandle);
	};
}

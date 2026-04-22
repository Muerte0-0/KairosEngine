#pragma once
#include "Engine.h"
#include "Panel.h"
#include <functional>

namespace Kairos
{
	class ContentBrowserPanel : public Panel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender() override;

		// Set by EditorLayer — called when a non-directory asset is double-clicked
		std::function<void(const std::filesystem::path&)> OnAssetDoubleClicked;

		// Set by EditorLayer — called on single-click of a non-directory asset
		std::function<void(const std::filesystem::path&)> OnAssetSelected;

		// Returns the currently single-clicked item (empty if none)
		const std::filesystem::path& GetSelectedPath() const { return m_SelectedPath; }

	private:
		void RenderFolderTree(const std::filesystem::path& directory);
		void DrawEmptySpaceContextMenu();
		void CreateNewAsset(const std::string& name, const std::string& extension);
		Ref<Texture> GetIconForPath(const std::filesystem::path& path) const;

		std::filesystem::path m_BaseDirectory;
		std::filesystem::path m_CurrentDirectory;
		std::filesystem::path m_SelectedPath;    // single-click selection
		std::filesystem::path m_RenamingPath;    // item currently being renamed (empty = none)
		char                  m_RenameBuffer[256] = {};  // edit buffer for inline rename

		// Icons
		Ref<Texture> m_DirectoryIcon;
		Ref<Texture> m_MeshIcon;
		Ref<Texture> m_TextureIcon;
		Ref<Texture> m_MaterialIcon;
		Ref<Texture> m_ShaderIcon;
		Ref<Texture> m_SceneIcon;
		Ref<Texture> m_FileIcon;      // fallback
	};
}

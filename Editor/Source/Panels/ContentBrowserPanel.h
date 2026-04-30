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

		// Set by EditorLayer — called when user picks "Instantiate" on a .prefab file
		std::function<void(const std::filesystem::path&)> OnPrefabInstantiate;

		// Returns the currently single-clicked item (empty if none)
		const std::filesystem::path& GetSelectedPath()    const { return m_SelectedPath; }
		const std::filesystem::path& GetCurrentDirectory() const { return m_CurrentDirectory; }

		// Triggers inline rename for the given path (must be inside m_CurrentDirectory).
		void BeginRename(const std::filesystem::path& path)
		{
			m_RenamingPath = path;
			std::string name = path.filename().string();
			strncpy_s(m_RenameBuffer, name.c_str(), sizeof(m_RenameBuffer) - 1);
			m_RenameFocusPending = true;
		}

	private:
		void RenderFolderTree(const std::filesystem::path& directory);
		void DrawEmptySpaceContextMenu();
		void CreateNewAsset(const std::string& name, const std::string& extension);
		Ref<Texture> GetIconForPath(const std::filesystem::path& path) const;

		std::filesystem::path m_BaseDirectory;
		std::filesystem::path m_CurrentDirectory;
		std::filesystem::path m_SelectedPath;    // single-click selection
		std::filesystem::path m_RenamingPath;    // item currently being renamed (empty = none)
		bool                  m_RenameFocusPending = false;
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

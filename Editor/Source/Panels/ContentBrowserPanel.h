#pragma once
#include <filesystem>
#include "Engine/Renderer/RHI/Resources/Texture.h"

namespace Engine
{	
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();
		
	private:
		void RenderFolderTree(const std::filesystem::path& directory);

		std::filesystem::path m_BaseDirectory;
		std::filesystem::path m_CurrentDirectory;
		
		Ref<Texture> m_DirectoryIcon;
		Ref<Texture> m_FileIcon;
	};
}

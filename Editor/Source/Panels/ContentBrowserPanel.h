#pragma once
#include <filesystem>
#include "Engine/Renderer/RHI/Resources/Texture.h"

namespace Engine
{	
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel() = default;

		void Init();

		void OnImGuiRender();
		
	private:
		std::filesystem::path m_CurrentDirectory;
		
		Ref<Texture> m_DirectoryIcon;
		Ref<Texture> m_FileIcon;
	};
}

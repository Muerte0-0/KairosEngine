#pragma once
#include <filesystem>
#include <string>

namespace Engine
{	
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();
		
		void OnImGuiRender();
		
	private:
		std::filesystem::path m_CurrentDirectory;
	};
}

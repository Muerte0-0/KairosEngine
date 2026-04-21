#pragma once
#include "Engine.h"
#include "EditorWindow.h"
#include <filesystem>

namespace Kairos
{
	class TextureEditorWindow : public EditorWindow
	{
	public:
		TextureEditorWindow(const std::filesystem::path& path, Engine::AssetHandle handle);

		void OnImGuiRender() override;
		const std::string& GetTitle() const override { return m_Title; }

	private:
		std::filesystem::path  m_SourcePath;
		Engine::AssetHandle    m_Handle;
		Ref<Texture>           m_Texture;

		bool                   m_sRGB    = false;
		bool                   m_GenMips = false;
	};
}

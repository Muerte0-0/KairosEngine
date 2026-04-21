#include "TextureEditorWindow.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "Engine/Assets/AssetManager.h"
#include "Engine/Assets/Editor/EditorAssetManager.h"
#include "Engine/Project/Project.h"

namespace Kairos
{
	static const char* TextureFormatToString(Engine::TextureFormat fmt)
	{
		switch (fmt)
		{
			case Engine::TextureFormat::RGBA8_UNorm:  return "RGBA8 UNorm";
			case Engine::TextureFormat::RGBA8_SRGB:   return "RGBA8 sRGB";
			case Engine::TextureFormat::BGRA8_SRGB:   return "BGRA8 sRGB";
			case Engine::TextureFormat::RGBA16_Float:  return "RGBA16 Float";
			case Engine::TextureFormat::RGBA32_Float:  return "RGBA32 Float";
			case Engine::TextureFormat::R8_UNorm:      return "R8 UNorm";
			default:                                   return "Unknown";
		}
	}

	TextureEditorWindow::TextureEditorWindow(const std::filesystem::path& path, Engine::AssetHandle handle)
		: m_SourcePath(path), m_Handle(handle)
	{
		m_Title   = path.stem().string();
		m_Texture = Engine::AssetManager::GetAsset<Texture>(handle);
	}

	void TextureEditorWindow::OnImGuiRender()
	{
		if (!m_Open) return;

		// Dock into outer dockspace next to Level Editor tab on first appearance
		if (m_OuterDockID != 0)
			ImGui::SetNextWindowDockID(m_OuterDockID, ImGuiCond_Appearing);

		std::string windowTitle = m_Title + "###TextureEditor_" + m_Title;

		if (!ImGui::Begin(windowTitle.c_str(), &m_Open, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return;
		}

		if (!m_Texture || !m_Texture->IsLoaded())
		{
			ImGui::TextColored({ 1, 0.3f, 0.3f, 1 }, "Texture failed to load.");
			ImGui::End();
			return;
		}

		// ── Layout: left half = preview | right half = top details + bottom settings
		const float totalWidth  = ImGui::GetContentRegionAvail().x;
		const float totalHeight = ImGui::GetContentRegionAvail().y;
		const float leftWidth   = totalWidth * 0.5f;
		const float rightWidth  = totalWidth - leftWidth - ImGui::GetStyle().ItemSpacing.x;

		// ── LEFT: Preview ──────────────────────────────────────────────────────
		ImGui::BeginChild("##TexPreview", ImVec2(leftWidth, totalHeight), false);
		{
			ImGui::SeparatorText("Preview");

			// Scale image to fit the child while preserving aspect ratio
			float imgW = (float)m_Texture->GetWidth();
			float imgH = (float)m_Texture->GetHeight();
			float avW  = ImGui::GetContentRegionAvail().x;
			float avH  = ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing();
			float scale = std::min(avW / imgW, avH / imgH);
			float dW = imgW * scale;
			float dH = imgH * scale;

			// Center horizontally
			float padX = (avW - dW) * 0.5f;
			if (padX > 0.f) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padX);

			ImGui::Image(m_Texture->GetTextureID(), { dW, dH }, { 0, 0 }, { 1, 1 });

			ImGui::TextDisabled("%ux%u", m_Texture->GetWidth(), m_Texture->GetHeight());
		}
		ImGui::EndChild();

		ImGui::SameLine();

		// ── RIGHT: stacked children ────────────────────────────────────────────
		ImGui::BeginChild("##TexRight", ImVec2(rightWidth, totalHeight), false);
		{
			const float halfH = totalHeight * 0.5f;

			// RIGHT-TOP: Asset Details
			ImGui::BeginChild("##TexDetails", ImVec2(0.f, halfH - ImGui::GetStyle().ItemSpacing.y * 0.5f), true);
			{
				ImGui::SeparatorText("Asset Details");
				ImGui::LabelText("File",   "%s", m_SourcePath.filename().string().c_str());
				ImGui::LabelText("Width",  "%u px", m_Texture->GetWidth());
				ImGui::LabelText("Height", "%u px", m_Texture->GetHeight());
				ImGui::LabelText("Format", "%s", TextureFormatToString(m_Texture->GetFormat()));
				ImGui::LabelText("Mip Levels", "1"); // TODO: expose mip count from Texture
			}
			ImGui::EndChild();

			// RIGHT-BOTTOM: Import Settings
			ImGui::BeginChild("##TexSettings", ImVec2(0.f, 0.f), true);
			{
				ImGui::SeparatorText("Import Settings");
				ImGui::Checkbox("sRGB",          &m_sRGB);
				ImGui::Checkbox("Generate Mips", &m_GenMips);

				ImGui::Spacing();
				if (ImGui::Button("Reimport", { -1.f, 0.f }))
				{
					// TODO: trigger reimport via EditorAssetManager once ImportSettings
					// block is wired into AssetSerializer
				}
			}
			ImGui::EndChild();
		}
		ImGui::EndChild();

		ImGui::End();
	}
}

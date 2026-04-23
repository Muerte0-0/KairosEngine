#include "ContentBrowserPanel.h"

#include "Engine.h"
#include "Engine/Assets/Editor/AssetImporter.h"
#include <fstream>
#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#include <commdlg.h>
#endif

namespace Kairos
{
	static Ref<Texture> TryLoadIcon(const std::string& path)
	{
		if (std::filesystem::exists(path))
			return Texture::Create(path);
		return nullptr;
	}

	ContentBrowserPanel::ContentBrowserPanel()
		: m_BaseDirectory(Project::GetAssetDirectory()), m_CurrentDirectory(m_BaseDirectory)
	{
		m_DirectoryIcon = Texture::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon      = Texture::Create("Resources/Icons/ContentBrowser/FileIcon.png");

		// Per-type icons — fall back to m_FileIcon if not found yet
		m_MeshIcon     = TryLoadIcon("Resources/Icons/ContentBrowser/MeshIcon.png");
		m_TextureIcon  = TryLoadIcon("Resources/Icons/ContentBrowser/TextureIcon.png");
		m_MaterialIcon = TryLoadIcon("Resources/Icons/ContentBrowser/MaterialIcon.png");
		m_ShaderIcon   = TryLoadIcon("Resources/Icons/ContentBrowser/ShaderIcon.png");
		m_SceneIcon    = TryLoadIcon("Resources/Icons/ContentBrowser/SceneIcon.png");
	}

	Ref<Texture> ContentBrowserPanel::GetIconForPath(const std::filesystem::path& path) const
	{
		using Engine::AssetImporter;
		using Engine::AssetType;

		AssetType type = AssetImporter::DeduceTypeFromPath(path);
		switch (type)
		{
			case AssetType::Mesh:     return m_MeshIcon     ? m_MeshIcon     : m_FileIcon;
			case AssetType::Texture:  return m_TextureIcon  ? m_TextureIcon  : m_FileIcon;
			case AssetType::Material: return m_MaterialIcon ? m_MaterialIcon : m_FileIcon;
			case AssetType::Shader:   return m_ShaderIcon   ? m_ShaderIcon   : m_FileIcon;
			case AssetType::Scene:    return m_SceneIcon    ? m_SceneIcon    : m_FileIcon;
			default:                  return m_FileIcon;
		}
	}

	// Recursive folder tree for left panel
	void ContentBrowserPanel::RenderFolderTree(const std::filesystem::path& directory)
	{
		if (!std::filesystem::exists(directory)) return;

		for (auto& entry : std::filesystem::directory_iterator(directory))
		{
			if (!entry.is_directory()) continue;

			const std::string name = entry.path().filename().string();
			const bool isCurrent   = (entry.path() == m_CurrentDirectory);

			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
				| ImGuiTreeNodeFlags_SpanFullWidth;

			if (isCurrent)
				flags |= ImGuiTreeNodeFlags_Selected;

			// Check if this folder has sub-folders
			bool hasSubDirs = false;
			for (auto& sub : std::filesystem::directory_iterator(entry.path()))
			{
				if (sub.is_directory()) { hasSubDirs = true; break; }
			}
			if (!hasSubDirs)
				flags |= ImGuiTreeNodeFlags_Leaf;

			bool open = ImGui::TreeNodeEx(name.c_str(), flags);

			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				m_CurrentDirectory = entry.path();

			if (open)
			{
				RenderFolderTree(entry.path());
				ImGui::TreePop();
			}
		}
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		// ── Header: Back button + path breadcrumb ──────────────────────────────
		bool canGoBack = (m_CurrentDirectory != m_BaseDirectory);
		if (!canGoBack) ImGui::BeginDisabled();

		if (ImGui::ArrowButton("##Back", ImGuiDir_Left))
			m_CurrentDirectory = m_CurrentDirectory.parent_path();

		if (!canGoBack) ImGui::EndDisabled();

		ImGui::Separator();

		static float padding       = 8.f;
		static float thumbnailSize = 96.f;

		// ── Main layout: left folder tree | right icon grid ───────────────────
		// Table with 2 columns: fixed left pane, stretching right pane
		if (ImGui::BeginTable("##CBLayout", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable,
			{ 0, ImGui::GetContentRegionAvail().y - 36}))   // leave room for settings footer
		{
			ImGui::TableSetupColumn("##FolderTree", ImGuiTableColumnFlags_WidthFixed, 200.f);
			ImGui::TableSetupColumn("##IconGrid",   ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableNextRow();

			// ── Left: folder tree ──
			ImGui::TableSetColumnIndex(0);
			ImGui::BeginChild("##FolderTreeChild", ImVec2(0, 0), false);
			RenderFolderTree(m_BaseDirectory);
			ImGui::EndChild();

			// ── Right: icon grid ──
			ImGui::TableSetColumnIndex(1);
			
			// Build relative path from base for display
			std::string displayPath = std::filesystem::relative(m_CurrentDirectory, m_BaseDirectory.parent_path()).string();
			// Replace backslashes for consistency
			for (auto& c : displayPath) if (c == '\\') c = '/';
			
			ImGui::TextUnformatted(displayPath.c_str());
			
			ImGui::Separator();
			
			ImGui::BeginChild("##IconGridChild", ImVec2(0, 0), false);

			float cellSize    = thumbnailSize + padding;
			float panelWidth  = ImGui::GetContentRegionAvail().x;
			int   columnCount = std::max(1, static_cast<int>(panelWidth / cellSize));

			if (std::filesystem::exists(m_CurrentDirectory))
			{
				// Sort: folders first, then files — alphabetical within each group
				std::vector<std::filesystem::directory_entry> dirs, files;
				for (auto& entry : std::filesystem::directory_iterator(m_CurrentDirectory))
				{
					if (!entry.is_directory() && entry.path().extension() == ".kasset")
						continue;
					(entry.is_directory() ? dirs : files).push_back(entry);
				}
				auto byName = [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
				{
					return a.path().filename() < b.path().filename();
				};
				std::sort(dirs.begin(),  dirs.end(),  byName);
				std::sort(files.begin(), files.end(), byName);

				std::vector<std::filesystem::directory_entry> sorted;
				sorted.reserve(dirs.size() + files.size());
				sorted.insert(sorted.end(), dirs.begin(),  dirs.end());
				sorted.insert(sorted.end(), files.begin(), files.end());

				ImGui::Columns(columnCount, nullptr, false);

				for (auto& directoryEntry : sorted)
				{
					const auto& path = directoryEntry.path();
					std::string filenameString = path.filename().string();

					ImGui::PushID(filenameString.c_str());

					Ref<Texture> icon = directoryEntry.is_directory() ? m_DirectoryIcon : GetIconForPath(path);

					bool isSelected = (path == m_SelectedPath);
					ImGui::PushStyleColor(ImGuiCol_Button, isSelected ? ImVec4(0.3f, 0.5f, 1.0f, 0.4f) : ImVec4(0, 0, 0, 0));
					ImGui::ImageButton(filenameString.c_str(), icon->GetTextureID(), { thumbnailSize, thumbnailSize });
					ImGui::PopStyleColor();
					
					if (ImGui::BeginDragDropSource())
					{
						const wchar_t* itemPath = path.c_str();
					// Specific payload per asset type so drop targets can filter precisely
						const char* payloadType = "CONTENT_BROWSER_ITEM";
						if (path.extension() == L".kscn")
							payloadType = "SCENE_ITEM";
						else if (path.extension() == L".obj"  || path.extension() == L".fbx" ||
						         path.extension() == L".gltf" || path.extension() == L".glb")
							payloadType = "MESH_ITEM";
						else if (path.extension() == L".png"  || path.extension() == L".jpg" ||
						         path.extension() == L".jpeg" || path.extension() == L".tga" ||
						         path.extension() == L".ktx")
							payloadType = "TEXTURE_ITEM";
						else if (path.extension() == L".kmat")
							payloadType = "MATERIAL_ITEM";

						ImGui::SetDragDropPayload(payloadType, itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);
						ImGui::TextUnformatted(filenameString.c_str());
						ImGui::EndDragDropSource();
					}

					// ── Double-click ──────────────────────────────────────────
					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						if (directoryEntry.is_directory())
							m_CurrentDirectory = path;
						else if (OnAssetDoubleClicked)
							OnAssetDoubleClicked(path);
					}

					// ── Single-click: select asset for Properties ─────────────
					if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						m_SelectedPath = path;
						if (!directoryEntry.is_directory() && OnAssetSelected)
							OnAssetSelected(path);
					}

					// ── Right-click context menu ──────────────────────────────
					std::string popupId = "##CBContext_" + filenameString;
					if (ImGui::BeginPopupContextItem(popupId.c_str()))
					{
						if (!directoryEntry.is_directory())
						{
							if (ImGui::MenuItem("Open"))
							{
								if (OnAssetDoubleClicked)
									OnAssetDoubleClicked(path);
							}
							ImGui::Separator();
							if (ImGui::MenuItem("Reimport"))
							{
								auto editorAM = Engine::Project::GetActive()->GetEditorAssetManager();
								Engine::AssetHandle handle = editorAM->ImportAsset(path);
								if (static_cast<uint64_t>(handle) != Engine::NullAssetHandle)
									editorAM->ReimportAsset(handle);
								else
									LOG(Engine::LogLevel::Warning, "Reimport: asset not registered — {}", path.string());
							}
							ImGui::Separator();
						}
						if (ImGui::MenuItem("Rename"))
						{
							m_RenamingPath = path;
							std::string stem = path.stem().string()
							                 + (directoryEntry.is_directory() ? "" : path.extension().string());
							strncpy_s(m_RenameBuffer, stem.c_str(), sizeof(m_RenameBuffer) - 1);
							ImGui::SetKeyboardFocusHere();
						}
						if (ImGui::MenuItem("Show in Explorer"))
						{
							std::filesystem::path target = directoryEntry.is_directory() ? path : path.parent_path();
#ifdef PLATFORM_WINDOWS
							std::string cmd = "explorer \"" + target.string() + "\"";
							system(cmd.c_str());
#endif
						}
						if (!directoryEntry.is_directory())
						{
							if (ImGui::MenuItem("Copy Path"))
								ImGui::SetClipboardText(path.string().c_str());
						}
						ImGui::EndPopup();
					}

					// ── F2 to begin rename ────────────────────────────────────
					if (path == m_SelectedPath && ImGui::IsKeyPressed(ImGuiKey_F2))
					{
						m_RenamingPath = path;
						std::string name = path.filename().string();
						strncpy_s(m_RenameBuffer, name.c_str(), sizeof(m_RenameBuffer) - 1);
					}

					// ── Label or inline rename field ──────────────────────────
					if (path == m_RenamingPath)
					{
						ImGui::SetNextItemWidth(thumbnailSize);
						ImGui::PushID("##RenameInput");
						if (ImGui::InputText("##Rename", m_RenameBuffer, sizeof(m_RenameBuffer),
							ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
						{
							// Commit rename
							std::string newName(m_RenameBuffer);
							if (!newName.empty() && newName != path.filename().string())
							{
								std::filesystem::path newPath = path.parent_path() / newName;
								std::error_code ec;
								std::filesystem::rename(path, newPath, ec);
								if (!ec)
								{
									if (m_SelectedPath == path) m_SelectedPath = newPath;
								}
								else
									LOG(Engine::LogLevel::Error, "Rename failed: {}", ec.message());
							}
							m_RenamingPath.clear();
						}
						// Esc cancels
						if (ImGui::IsKeyPressed(ImGuiKey_Escape))
							m_RenamingPath.clear();
						ImGui::PopID();
					}
					else
					{
						ImGui::TextUnformatted(filenameString.c_str());
					}
					ImGui::NextColumn();
					ImGui::PopID();
				}
				
				ImGui::Columns(1);
				
				// ── Empty-space right-click ─────────────────────────────────────
				if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) &&
				    ImGui::IsMouseClicked(ImGuiMouseButton_Right) &&
				    !ImGui::IsAnyItemHovered())
				{
					ImGui::OpenPopup("##CBEmptyPopup");
				}
				DrawEmptySpaceContextMenu();
				
				ImGui::Dummy({0, (ImGui::GetContentRegionAvail().y - 96)});
			
				// ── Settings ────────────────────────────────────────────────────
				ImGui::SeparatorText("Settings");
				ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16.f, 512.f);
				ImGui::SliderFloat("Padding",        &padding,       0.f,  32.f);
			}
			
			ImGui::EndChild(); // Icon Grid Child

			ImGui::EndTable();
		}
		ImGui::End();
	}

	// =====================================================================
	// CreateNewAsset
	// Creates a uniquely-named stub file in m_CurrentDirectory.
	// AssetImporter::ImportMaterial handles first-open initialisation.
	// =====================================================================
	void ContentBrowserPanel::CreateNewAsset(const std::string& baseName,
	                                          const std::string& extension)
	{
		std::filesystem::path candidate = m_CurrentDirectory / (baseName + extension);
		int suffix = 0;
		while (std::filesystem::exists(candidate))
			candidate = m_CurrentDirectory / (baseName + std::to_string(++suffix) + extension);

		// Empty file — ImportMaterial / ImportScene will fill it on first open
		{ std::ofstream f(candidate); }

		LOG(Engine::LogLevel::Info, "ContentBrowser: created '{}'.", candidate.string());
	}


	// =====================================================================
	// DrawEmptySpaceContextMenu
	// =====================================================================
	void ContentBrowserPanel::DrawEmptySpaceContextMenu()
	{
		if (!ImGui::BeginPopup("##CBEmptyPopup")) return;

		// ── Import ────────────────────────────────────────────────────────
		if (ImGui::MenuItem("  Import Asset..."))
		{
#ifdef PLATFORM_WINDOWS
			char filename[MAX_PATH] = {};
			OPENFILENAMEA ofn   = {};
			ofn.lStructSize     = sizeof(ofn);
			ofn.lpstrFile       = filename;
			ofn.nMaxFile        = MAX_PATH;
			ofn.lpstrFilter     =
				"All Supported\0*.obj;*.fbx;*.gltf;*.glb;*.png;*.jpg;*.jpeg;*.tga;*.ktx\0"
				"Mesh\0*.obj;*.fbx;*.gltf;*.glb\0"
				"Texture\0*.png;*.jpg;*.jpeg;*.tga;*.ktx\0"
				"All Files\0*.*\0";
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

			if (GetOpenFileNameA(&ofn))
			{
				std::filesystem::path src(filename);
				std::filesystem::path dst = m_CurrentDirectory / src.filename();
				std::error_code ec;
				if (!std::filesystem::exists(dst))
					std::filesystem::copy_file(src, dst, ec);
				if (!ec && OnAssetDoubleClicked)
					OnAssetDoubleClicked(dst);
			}
#endif
			ImGui::CloseCurrentPopup();
		}

		ImGui::Separator();
		ImGui::SeparatorText("Create");

		if (ImGui::MenuItem("  Material  (.kmat)"))
		{
			CreateNewAsset("NewMaterial", ".kmat");
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItem("  Scene     (.kscn)"))
		{
			CreateNewAsset("NewScene", ".kscn");
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItem("  Folder"))
		{
			std::filesystem::path newDir = m_CurrentDirectory / "NewFolder";
			int suffix = 0;
			while (std::filesystem::exists(newDir))
				newDir = m_CurrentDirectory / ("NewFolder" + std::to_string(++suffix));
			std::filesystem::create_directory(newDir);
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

} // namespace Kairos

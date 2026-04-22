#pragma once
#include "Engine.h"
#include "EditorWindow.h"
#include <ImNodeFlow.h>
#include <filesystem>
#include <memory>
#include <string>

namespace Kairos
{
    // Forward-declare per-node ImNodeFlow wrappers (defined in .cpp)
    struct INF_PBROutputNode;
    struct INF_TextureSampleNode;
    struct INF_ConstantVec3Node;
    struct INF_MultiplyNode;
    struct INF_LerpNode;

    class MaterialEditorWindow : public EditorWindow
    {
    public:
        // Open an existing .kmat file
        explicit MaterialEditorWindow(const std::filesystem::path& path, AssetHandle handle);

        // Create a new empty material (no saved file yet)
        MaterialEditorWindow();

        void OnImGuiRender() override;
        const std::string& GetTitle() const override { return m_Title; }

        void SetContentBrowserSelection(const std::filesystem::path& p) { m_CBSelectedPath = p; }
        
    private:
        // --- Toolbar helpers ---
        void DrawToolbar();
        void DrawNodeCanvas();
        void DrawParamsSidebar();

        // --- Right-click "Add node" popup ---
        void DrawAddNodePopup();

        // --- Build / rebuild ImNodeFlow nodes from graph data ---
        void RebuildCanvasFromGraph();

        // Add a single node to the canvas at grid position
        void AddNodeToCanvas(Engine::MaterialNode* node);

        // Sync canvas link state → graph (called after ImNodeFlow reports new/deleted links)
        void SyncLinksFromCanvas();

        // Save back to .kmat
        void Save();

    private:
        std::filesystem::path   m_Path;
        Engine::AssetHandle     m_Handle = Engine::AssetHandle(Engine::NullAssetHandle);
        bool                    m_Dirty  = false;

        std::filesystem::path   m_CBSelectedPath;
        
        Engine::MaterialGraph   m_Graph;
        ImFlow::ImNodeFlow      m_Canvas{ "MaterialEditor" };

        // Map graph node ID → live ImNodeFlow shared_ptr (for selection / param editing)
        std::unordered_map<uint32_t, std::shared_ptr<ImFlow::BaseNode>> m_CanvasNodes;

        // Currently selected node ID (0 = none)
        uint32_t m_SelectedNodeID    = 0;
        bool     m_OpenAddNodePopup  = false;   // set by toolbar button, consumed by DrawNodeCanvas
    };
}
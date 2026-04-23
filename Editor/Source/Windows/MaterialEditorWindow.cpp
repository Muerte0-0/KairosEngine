#include "MaterialEditorWindow.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "Engine/Debugging/Log.h"
#include "Engine/Assets/AssetManager.h"
#include "Engine/Assets/Editor/EditorAssetManager.h"
#include "Engine/Project/Project.h"

// ================================================================
// ImNodeFlow node wrappers
// Each wrapper holds a back-pointer to the graph node it represents
// and implements draw() to show pins + inline params.
// ================================================================
namespace Kairos
{
    // ----------------------------------------------------------------
    // Colour palette for pin types
    // ----------------------------------------------------------------
    static ImU32 PinColor(Engine::PinType t)
    {
        switch (t) {
            case Engine::PinType::Float:     return IM_COL32(180, 220, 120, 255);
            case Engine::PinType::Vec2:      return IM_COL32( 87, 155, 185, 255);
            case Engine::PinType::Vec3:      return IM_COL32( 90, 191,  93, 255);
            case Engine::PinType::Vec4:      return IM_COL32(191, 134,  90, 255);
            case Engine::PinType::Texture2D: return IM_COL32(191,  90, 191, 255);
            default:                         return IM_COL32(255, 255, 255, 255);
        }
    }
    
    static std::shared_ptr<ImFlow::PinStyle> MakePinStyle(Engine::PinType t)
    {
        return std::make_shared<ImFlow::PinStyle>(PinColor(t), 0, 4.f, 4.67f, 3.7f, 1.f);
    }
    
    
    // ----------------------------------------------------------------
    // Generic graph-node wrapper — works for any MaterialNode
    // ----------------------------------------------------------------
    class GraphNodeWrapper : public ImFlow::BaseNode
    {
    public:
        explicit GraphNodeWrapper(Engine::MaterialNode* graphNode)
            : m_GraphNode(graphNode)
        {
            setTitle(graphNode->Name);
            setStyle(ImFlow::NodeStyle::cyan());

            // Register pins statically so getIns()/getOuts() work for link restore/sync
            for (auto& pin : graphNode->Inputs)
                addIN_uid(pin.ID, pin.Name, pin.DefaultValue,
                    [](ImFlow::Pin*, ImFlow::Pin*){ return true; },
                    MakePinStyle(pin.Type));

            for (auto& pin : graphNode->Outputs)
            {
                glm::vec4 defVal = pin.DefaultValue;
                addOUT_uid<glm::vec4>(pin.ID, pin.Name,
                    MakePinStyle(pin.Type))
                    ->behaviour([defVal](){ return defVal; });
            }
        }
    
        void draw() override {} // pins already registered in ctor

        Engine::MaterialNode* GetGraphNode() const { return m_GraphNode; }

    private:
        Engine::MaterialNode* m_GraphNode = nullptr;
    };
    
    
    // ================================================================
    // MaterialEditorWindow — constructors
    // ================================================================
    MaterialEditorWindow::MaterialEditorWindow(const std::filesystem::path& path, Engine::AssetHandle handle) : m_Path(path), m_Handle(handle)
    {
        m_Title = path.stem().string();
        if (!m_Graph.LoadFromFile(path))
            m_Graph.InitDefault();
        RebuildCanvasFromGraph();
    }
    
    MaterialEditorWindow::MaterialEditorWindow()
    {
        m_Title = "Untitled Material";
        m_Graph.InitDefault();
        RebuildCanvasFromGraph();
        m_Dirty = true;
    }
    
    void MaterialEditorWindow::OnImGuiRender()
    {
        if (!m_Open) return;

        if (m_OuterDockID != 0)
            ImGui::SetNextWindowDockID(m_OuterDockID, ImGuiCond_Appearing);

        // Unique stable ID so multiple material windows can coexist
        std::string windowTitle = m_Title + (m_Dirty ? " *" : "") + "###MatEd_" + m_Title;
        ImGui::SetNextWindowSize(ImVec2(1200.f, 720.f), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin(windowTitle.c_str(), &m_Open, ImGuiWindowFlags_NoCollapse))
        {
            ImGui::End();
            return;
        }

        DrawToolbar();
        ImGui::Separator();

        // Split: canvas left | params sidebar right
        constexpr float kSidebarW = 400.f;
        float canvasW = ImGui::GetContentRegionAvail().x - kSidebarW - 6.f;
        float height  = ImGui::GetContentRegionAvail().y;

        ImGui::BeginChild("##MatCanvas", ImVec2(canvasW, height), false,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        DrawNodeCanvas();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("##MatParams", ImVec2(kSidebarW, height), true);
        DrawParamsSidebar();
        ImGui::EndChild();

        ImGui::End();
    }
    
    // ================================================================
    // RebuildCanvasFromGraph — push all graph nodes onto the canvas
    // ================================================================
    void MaterialEditorWindow::RebuildCanvasFromGraph()
    {
        m_CanvasNodes.clear();
        for (auto& node : m_Graph.Nodes)
        {
            auto wrapper = m_Canvas.addNode<GraphNodeWrapper>(
                ImVec2(node->Position.x, node->Position.y),
                node.get());
            m_CanvasNodes[node->ID] = wrapper;
        }

        // Restore links: for each saved MaterialLink, find the canvas pins by
        // hashed UID and call createLink() so ImNodeFlow owns the connection.
        for (auto& link : m_Graph.Links)
        {
            ImFlow::PinUID fromHash = std::hash<uint32_t>{}(link.FromPin);
            ImFlow::PinUID toHash   = std::hash<uint32_t>{}(link.ToPin);

            ImFlow::Pin* outPin = nullptr;
            ImFlow::Pin* inPin  = nullptr;

            for (auto& [nodeID, wrapper] : m_CanvasNodes)
            {
                for (auto& p : wrapper->getOuts())
                    if (p->getUid() == fromHash) { outPin = p.get(); }

                for (auto& p : wrapper->getIns())
                    if (p->getUid() == toHash)   { inPin  = p.get(); }
            }

            if (outPin && inPin)
                outPin->createLink(inPin);
        }
    }
    
    void MaterialEditorWindow::AddNodeToCanvas(Engine::MaterialNode* node)
    {
        auto wrapper = m_Canvas.addNode<GraphNodeWrapper>(
            ImVec2(node->Position.x, node->Position.y),
            node);
        m_CanvasNodes[node->ID] = wrapper;
    }
    
    void MaterialEditorWindow::DrawToolbar()
    {
        if (ImGui::Button("  Save  "))
            Save();

        ImGui::SameLine();

        if (ImGui::Button("  Add Node  "))
            m_OpenAddNodePopup = true;          // signal — popup opened from canvas child

        ImGui::SameLine();

        if (ImGui::Button("  Rebuild  "))
            RebuildCanvasFromGraph();

        ImGui::SameLine();

        // Dirty indicator
        ImVec4 indicatorCol = m_Dirty
            ? ImVec4(1.0f, 0.6f, 0.1f, 1.0f)   // amber = unsaved
            : ImVec4(0.4f, 0.9f, 0.4f, 1.0f);   // green  = saved
        ImGui::TextColored(indicatorCol, m_Dirty ? "●  Unsaved" : "●  Saved");
    }
    
    // ================================================================
    // DrawNodeCanvas
    // ================================================================
    void MaterialEditorWindow::DrawNodeCanvas()
    {
        m_Canvas.update();

        // Poll ImNodeFlow selection — whichever node reports isSelected() wins
        for (auto& [id, wrapper] : m_CanvasNodes)
        {
            if (wrapper->isSelected())
                m_SelectedNodeID = id;
        }

        // Toolbar "Add Node" button deferred open — must be called inside the canvas child
        if (m_OpenAddNodePopup)
        {
            ImGui::OpenPopup("##AddNodePopup");
            m_OpenAddNodePopup = false;
        }

        // Right-click on empty canvas space → same popup
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Right) &&
            !ImGui::IsAnyItemHovered())
        {
            ImGui::OpenPopup("##AddNodePopup");
        }
        DrawAddNodePopup();

        // Detect deleted nodes
        for (auto it = m_CanvasNodes.begin(); it != m_CanvasNodes.end(); )
        {
            if (it->second->toDestroy())
            {
                m_Graph.RemoveNode(it->first);
                if (m_SelectedNodeID == it->first) m_SelectedNodeID = 0;
                it = m_CanvasNodes.erase(it);
                m_Dirty = true;
            }
            else ++it;
        }

        // Sync node positions back to graph every frame (cheap)
        for (auto& [id, wrapper] : m_CanvasNodes)
        {
            if (auto* gn = m_Graph.FindNode(id))
            {
                ImVec2 pos = wrapper->getPos();
                gn->Position = { pos.x, pos.y };
            }
        }

        // DEL key — destroy selected node (toDestroy() sweep above handles cleanup)
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) &&
            ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            for (auto& [id, wrapper] : m_CanvasNodes)
            {
                if (wrapper->isSelected())
                    wrapper->destroy();
            }
        }
    }
    
    
    // ================================================================
    // DrawParamsSidebar
    // ================================================================
    void MaterialEditorWindow::DrawParamsSidebar()
    {
        ImGui::SeparatorText("Node Parameters");

        if (m_SelectedNodeID == 0)
        {
            ImGui::Spacing();
            ImGui::TextDisabled("  Click a node to inspect it.");
            return;
        }

        Engine::MaterialNode* node = m_Graph.FindNode(m_SelectedNodeID);
        if (!node) { m_SelectedNodeID = 0; return; }

        // Node name + type badge
        ImGui::Text("%s", node->Name.c_str());
        ImGui::SameLine();
        ImGui::TextDisabled("(%s)", node->GetTypeName().c_str());
        ImGui::Spacing();

        // ----------------------------------------------------------------
        // Type-specific section
        // ----------------------------------------------------------------
        if (node->GetTypeName() == "TextureSample")
        {
            auto* tsn = static_cast<Engine::TextureSampleNode*>(node);

            ImGui::SeparatorText("Texture");

            // ── Texture preview ──────────────────────────────────────────
            bool hasTexture = static_cast<uint64_t>(tsn->TextureHandle) != Engine::NullAssetHandle;
            if (hasTexture)
            {
                auto texAsset = Engine::AssetManager::GetAsset<Engine::Texture>(tsn->TextureHandle);
                if (texAsset)
                {
                    float previewW = ImGui::GetContentRegionAvail().x;
                    ImGui::Image(texAsset->GetTextureID(), ImVec2(previewW, previewW));
                }
                else
                {
                    ImGui::TextColored(ImVec4(1,0.4f,0.4f,1), "  Asset not loaded");
                }
            }
            else
            {
                // Empty placeholder rect
                ImVec2 pos  = ImGui::GetCursorScreenPos();
                float  sz   = ImGui::GetContentRegionAvail().x;
                ImGui::GetWindowDrawList()->AddRectFilled(pos, { pos.x + sz, pos.y + sz * 0.5f },
                    IM_COL32(40, 40, 40, 200), 4.f);
                ImGui::GetWindowDrawList()->AddRect(pos, { pos.x + sz, pos.y + sz * 0.5f },
                    IM_COL32(100, 100, 100, 200), 4.f);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + sz * 0.5f);
                ImGui::TextDisabled("  No texture assigned");
            }

            ImGui::Spacing();

            // ── Assign strip ─────────────────────────────────────────────
            // [← Assign from CB]  [drag-drop target strip]
            bool cbIsTexture = false;
            if (!m_CBSelectedPath.empty())
            {
                auto ext = m_CBSelectedPath.extension();
                cbIsTexture = (ext == ".png" || ext == ".jpg" || ext == ".jpeg"
                            || ext == ".tga" || ext == ".ktx");
            }

            if (!cbIsTexture) ImGui::BeginDisabled();
            if (ImGui::Button("  \u2190 Assign from Content Browser  "))
            {
                auto editorAM = Engine::Project::GetActive()->GetEditorAssetManager();
                auto h = editorAM->ImportAsset(m_CBSelectedPath);
                if (static_cast<uint64_t>(h) != Engine::NullAssetHandle)
                {
                    tsn->TextureHandle = h;
                    m_Dirty = true;
                }
            }
            if (!cbIsTexture) ImGui::EndDisabled();

            if (!cbIsTexture)
                ImGui::TextDisabled("  (select a texture in Content Browser)");
            else
                ImGui::TextDisabled("  %s", m_CBSelectedPath.filename().string().c_str());

            // ── Drag-drop target ──────────────────────────────────────────
            ImGui::Spacing();
            ImGui::TextDisabled("  or drag a texture here:");
            ImGui::Button("  Drop Texture Here  ", ImVec2(ImGui::GetContentRegionAvail().x, 0));
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("TEXTURE_ITEM"))
                {
                    std::wstring wpath(static_cast<const wchar_t*>(p->Data),
                                       p->DataSize / sizeof(wchar_t));
                    auto editorAM = Engine::Project::GetActive()->GetEditorAssetManager();
                    auto h = editorAM->ImportAsset(std::filesystem::path(wpath));
                    if (static_cast<uint64_t>(h) != Engine::NullAssetHandle)
                    {
                        tsn->TextureHandle = h;
                        m_Dirty = true;
                    }
                }
                ImGui::EndDragDropTarget();
            }

            // ── Handle display ─────────────────────────────────────────────
            if (hasTexture)
            {
                ImGui::Spacing();
                ImGui::TextDisabled("  Handle: 0x%016llX",
                    static_cast<unsigned long long>(static_cast<uint64_t>(tsn->TextureHandle)));
            }
        }
        else if (node->GetTypeName() == "ConstantVec3")
        {
            auto* cvn = static_cast<Engine::ConstantVec3Node*>(node);
            ImGui::SeparatorText("Value");
            if (ImGui::ColorEdit3("##Vec3Val", &cvn->Value.x,
                ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR))
                m_Dirty = true;
            // Also expose as drag-float for non-colour use cases
            if (ImGui::DragFloat3("XYZ", &cvn->Value.x, 0.01f))
                m_Dirty = true;
        }
        else if (node->GetTypeName() == "PBROutput")
        {
            ImGui::SeparatorText("Info");
            ImGui::TextWrapped("This is the final PBR output node. "
                "Connect BaseColor, Metallic, Roughness, Normal, Emissive "
                "and AmbientOcclusion from the graph above.");
        }

        // ----------------------------------------------------------------
        // Input pin defaults — shown for every node type
        // ----------------------------------------------------------------
        if (!node->Inputs.empty())
        {
            ImGui::Spacing();
            ImGui::SeparatorText("Input Defaults");
            ImGui::TextDisabled("Used when pin has no incoming connection.");
            ImGui::Spacing();

            for (auto& pin : node->Inputs)
            {
                // Skip Texture2D pins — no meaningful default scalar
                if (pin.Type == Engine::PinType::Texture2D) continue;

                ImGui::PushID(static_cast<int>(pin.ID));
                switch (pin.Type)
                {
                    case Engine::PinType::Float:
                        if (ImGui::DragFloat(pin.Name.c_str(), &pin.DefaultValue.x, 0.01f))
                            m_Dirty = true;
                        break;
                    case Engine::PinType::Vec2:
                        if (ImGui::DragFloat2(pin.Name.c_str(), &pin.DefaultValue.x, 0.01f))
                            m_Dirty = true;
                        break;
                    case Engine::PinType::Vec3:
                        if (ImGui::ColorEdit3(pin.Name.c_str(), &pin.DefaultValue.x,
                            ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR))
                            m_Dirty = true;
                        break;
                    case Engine::PinType::Vec4:
                        if (ImGui::ColorEdit4(pin.Name.c_str(), &pin.DefaultValue.x,
                            ImGuiColorEditFlags_Float))
                            m_Dirty = true;
                        break;
                    default: break;
                }
                ImGui::PopID();
            }
        }
    }
    
    
    // ================================================================
    // DrawAddNodePopup
    // ================================================================
    void MaterialEditorWindow::DrawAddNodePopup()
    {
        if (!ImGui::BeginPopup("##AddNodePopup")) return;

        // Spawn at mouse position converted to grid space
        ImVec2 mouseScreen = ImGui::GetMousePosOnOpeningCurrentPopup();
        ImVec2 canvasOrigin = m_Canvas.getPos();
        ImVec2 scroll       = m_Canvas.getScroll();
        glm::vec2 spawnPos  = {
            mouseScreen.x - canvasOrigin.x - scroll.x,
            mouseScreen.y - canvasOrigin.y - scroll.y
        };

        auto addNode = [&](auto nodePtr)
        {
            m_Graph.Nodes.push_back(nodePtr);
            AddNodeToCanvas(nodePtr.get());
            m_Dirty = true;
            ImGui::CloseCurrentPopup();
        };

        ImGui::SeparatorText("Math");
        if (ImGui::MenuItem("Multiply"))
            addNode(Engine::MultiplyNode::Create(m_Graph.NextNodeID(), spawnPos));
        if (ImGui::MenuItem("Lerp"))
            addNode(Engine::LerpNode::Create(m_Graph.NextNodeID(), spawnPos));

        ImGui::SeparatorText("Constant");
        if (ImGui::MenuItem("Vec3 Constant"))
            addNode(Engine::ConstantVec3Node::Create(m_Graph.NextNodeID(), spawnPos));

        ImGui::SeparatorText("Texture");
        if (ImGui::MenuItem("Texture Sample"))
            addNode(Engine::TextureSampleNode::Create(m_Graph.NextNodeID(), spawnPos));

        ImGui::SeparatorText("Output");
        if (ImGui::MenuItem("PBR Output"))
            addNode(Engine::PBROutputNode::Create(m_Graph.NextNodeID(), spawnPos));

        ImGui::EndPopup();
    }
    
    // ================================================================
    // SyncLinksFromCanvas — reconstruct m_Graph.Links from live canvas state
    // ================================================================
    void MaterialEditorWindow::SyncLinksFromCanvas()
    {
        m_Graph.Links.clear();

        for (auto& [nodeID, wrapper] : m_CanvasNodes)
        {
            for (auto& pin : wrapper->getIns())
            {
                if (!pin->isConnected()) continue;

                auto linkWeak = pin->getLink();
                auto link     = linkWeak.lock();
                if (!link) continue;

                // left() = output pin on the source node
                ImFlow::PinUID outUID = link->left()->getUid();
                ImFlow::PinUID inUID  = pin->getUid();

                // Match UIDs back to graph pin IDs
                // ImNodeFlow hashes via std::hash<uint32_t>{}, so we can search by comparing
                uint32_t fromPinID = 0, toPinID = 0;
                bool foundFrom = false, foundTo = false;

                for (auto& node : m_Graph.Nodes)
                {
                    for (auto& gpin : node->Outputs)
                    {
                        if (std::hash<uint32_t>{}(gpin.ID) == outUID)
                        {
                            fromPinID = gpin.ID;
                            foundFrom = true;
                        }
                    }
                    for (auto& gpin : node->Inputs)
                    {
                        if (std::hash<uint32_t>{}(gpin.ID) == inUID)
                        {
                            toPinID = gpin.ID;
                            foundTo = true;
                        }
                    }
                }

                if (foundFrom && foundTo)
                {
                    Engine::MaterialLink ml;
                    ml.ID      = m_Graph.NextLinkID();
                    ml.FromPin = fromPinID;
                    ml.ToPin   = toPinID;
                    m_Graph.Links.push_back(ml);
                }
            }
        }
    }
    
    // ================================================================
    // Save
    // ================================================================
    void MaterialEditorWindow::Save()
    {
        if (m_Path.empty())
        {
            LOG(LogLevel::Warning, "MaterialEditorWindow::Save — no path set, cannot save.");
            return;
        }

        // Sync canvas connection state into graph before serialising
        SyncLinksFromCanvas();
        if (m_Graph.SaveToFile(m_Path))
        {
            m_Dirty = false;
            LOG(LogLevel::Info, "Material saved: {0}", m_Path.string());

            // Mark the live asset dirty so MeshRenderSystem recompiles on next draw.
            if (static_cast<uint64_t>(m_Handle) != Engine::NullAssetHandle)
            {
                auto asset = Engine::AssetManager::GetAsset<Engine::MaterialAsset>(m_Handle);
                if (asset)
                {
                    asset->Graph    = m_Graph; // push editor copy back
                    asset->IsDirty  = true;    // compiler picks this up next frame
                }
            }
        }
    }
    
}

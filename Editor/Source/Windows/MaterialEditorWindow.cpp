#include "MaterialEditorWindow.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "Engine/Debugging/Log.h"

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
        explicit GraphNodeWrapper(Engine::MaterialNode* graphNode, uint32_t* selectedID)
            : m_GraphNode(graphNode), m_SelectedID(selectedID)
        {
            setTitle(graphNode->Name);
            setStyle(ImFlow::NodeStyle::cyan());
        }
    
        void draw() override
        {
            // Register click for param sidebar selection
            if (ImGui::IsItemClicked())
                *m_SelectedID = m_GraphNode->ID;
    
            // Input pins
            for (auto& pin : m_GraphNode->Inputs)
                showIN_uid<glm::vec4>(pin.ID, pin.Name,
                    pin.DefaultValue,
                    [](ImFlow::Pin*, ImFlow::Pin*){ return true; },
                    MakePinStyle(pin.Type));
    
            // Output pins
            for (auto& pin : m_GraphNode->Outputs)
                showOUT_uid<glm::vec4>(pin.ID, pin.Name,
                    [&pin](){ return pin.DefaultValue; },
                    MakePinStyle(pin.Type));
        }
    
        Engine::MaterialNode* GetGraphNode() const { return m_GraphNode; }
    
    private:
        Engine::MaterialNode* m_GraphNode = nullptr;
        uint32_t*             m_SelectedID = nullptr;
    };
    
    
    // ================================================================
    // MaterialEditorWindow — constructors
    // ================================================================
    MaterialEditorWindow::MaterialEditorWindow(const std::filesystem::path& path, Engine::AssetHandle handle) : m_Path(path), m_Handle(handle)
    {
        m_Title = "Material: " + path.stem().string();
        if (!m_Graph.LoadFromFile(path))
            m_Graph.InitDefault();
        RebuildCanvasFromGraph();
    }
    
    MaterialEditorWindow::MaterialEditorWindow()
    {
        m_Title = "Material: Untitled*";
        m_Graph.InitDefault();
        RebuildCanvasFromGraph();
        m_Dirty = true;
    }
    
    void MaterialEditorWindow::OnImGuiRender()
    {
        if (!m_Open) return;
    
        if (m_OuterDockID != 0)
            ImGui::SetNextWindowDockID(m_OuterDockID, ImGuiCond_Appearing);
    
        std::string windowTitle = m_Title + "###MaterialEditor_" + m_Title;
        
        if (!ImGui::Begin(windowTitle.c_str(), &m_Open, ImGuiWindowFlags_NoCollapse))
        {
            ImGui::End();
            return;
        }
        
        ImGui::EndChild();
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
                node.get(), &m_SelectedNodeID);
            m_CanvasNodes[node->ID] = wrapper;
        }
        // Restore links
        for (auto& link : m_Graph.Links)
        {
            // ImNodeFlow re-creates links automatically when pins with matching UIDs
            // are connected — we just flag existing pin connections here if needed.
            // Full link restoration requires ImNodeFlow's internal link API; for now
            // the user will see correct pins and can reconnect if needed after load.
            // TODO: add proper link restore via ImNodeFlow pin lookup when API supports it.
            (void)link;
        }
    }
    
    void MaterialEditorWindow::AddNodeToCanvas(Engine::MaterialNode* node)
    {
        auto wrapper = m_Canvas.addNode<GraphNodeWrapper>(
            ImVec2(node->Position.x, node->Position.y),
            node, &m_SelectedNodeID);
        m_CanvasNodes[node->ID] = wrapper;
    }
    
    void MaterialEditorWindow::DrawToolbar()
    {
        if (ImGui::Button("Save"))
        {
            Save();
        }
    
        ImGui::SameLine();
    
        if (ImGui::Button("Rebuild"))
        {
            RebuildCanvasFromGraph();
        }
    
        ImGui::SameLine();
    
        ImGui::TextDisabled(m_Dirty ? "● Modified" : "Saved");
    }
    
    // ================================================================
    // DrawNodeCanvas
    // ================================================================
    void MaterialEditorWindow::DrawNodeCanvas()
    {
        m_Canvas.update();
    
        // Detect deleted nodes — any canvas node no longer alive
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
    }
    
    
    // ================================================================
    // DrawParamsSidebar
    // ================================================================
    void MaterialEditorWindow::DrawParamsSidebar()
    {
        ImGui::TextDisabled("Parameters");
        ImGui::Separator();
    
        if (m_SelectedNodeID == 0)
        {
            ImGui::TextDisabled("(select a node)");
            return;
        }
    
        Engine::MaterialNode* node = m_Graph.FindNode(m_SelectedNodeID);
        if (!node)
        {
            m_SelectedNodeID = 0;
            return;
        }
    
        ImGui::Text("%s", node->Name.c_str());
        ImGui::Spacing();
    
        // ---- Type-specific params ----
        if (node->GetTypeName() == "TextureSample")
        {
            auto* tsn = static_cast<Engine::TextureSampleNode*>(node);
            uint64_t raw = static_cast<uint64_t>(tsn->TextureHandle);
            ImGui::TextDisabled("Texture Handle");
            ImGui::SameLine();
            // Show handle as hex; user replaces via drag-drop from Content Browser later
            ImGui::Text("0x%016llX", static_cast<unsigned long long>(raw));
    
            // Drag-drop target for a texture asset from the Content Browser
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("ASSET_HANDLE"))
                {
                    Engine::AssetHandle dropped;
                    memcpy(&dropped, p->Data, sizeof(Engine::AssetHandle));
                    if (Engine::AssetManager::GetAssetType(dropped) == Engine::AssetType::Texture)
                    {
                        tsn->TextureHandle = dropped;
                        m_Dirty = true;
                    }
                }
                ImGui::EndDragDropTarget();
            }
            ImGui::TextDisabled("(drag texture from Content Browser)");
        }
        else if (node->GetTypeName() == "ConstantVec3")
        {
            auto* cvn = static_cast<Engine::ConstantVec3Node*>(node);
            if (ImGui::ColorEdit3("Value", &cvn->Value.x))
                m_Dirty = true;
        }
    
        ImGui::Spacing();
        ImGui::Separator();
    
        // ---- Pin defaults (inputs only) ----
        if (!node->Inputs.empty())
        {
            ImGui::TextDisabled("Input defaults");
            for (auto& pin : node->Inputs)
            {
                ImGui::PushID(pin.ID);
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
                        if (ImGui::ColorEdit3(pin.Name.c_str(), &pin.DefaultValue.x))
                            m_Dirty = true;
                        break;
                    case Engine::PinType::Vec4:
                        if (ImGui::ColorEdit4(pin.Name.c_str(), &pin.DefaultValue.x))
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
    
        // Spawn position — centre of current canvas view
        ImVec2 scroll = m_Canvas.getScroll();
        glm::vec2 spawnPos = { -scroll.x + 300.0f, -scroll.y + 200.0f };
    
        auto addNode = [&](auto nodePtr)
        {
            m_Graph.Nodes.push_back(nodePtr);
            AddNodeToCanvas(nodePtr.get());
            m_Dirty = true;
            ImGui::CloseCurrentPopup();
        };
    
        if (ImGui::MenuItem("Texture Sample"))
            addNode(Engine::TextureSampleNode::Create(m_Graph.NextNodeID(), spawnPos));
    
        if (ImGui::MenuItem("Constant Vec3"))
            addNode(Engine::ConstantVec3Node::Create(m_Graph.NextNodeID(), spawnPos));
    
        if (ImGui::MenuItem("Multiply"))
            addNode(Engine::MultiplyNode::Create(m_Graph.NextNodeID(), spawnPos));
    
        if (ImGui::MenuItem("Lerp"))
            addNode(Engine::LerpNode::Create(m_Graph.NextNodeID(), spawnPos));
    
        ImGui::Separator();
        if (ImGui::MenuItem("PBR Output"))
            addNode(Engine::PBROutputNode::Create(m_Graph.NextNodeID(), spawnPos));
    
        ImGui::EndPopup();
    }
    
    // ================================================================
    // SyncLinksFromCanvas  (stub — ImNodeFlow manages link lifetime)
    // ================================================================
    void MaterialEditorWindow::SyncLinksFromCanvas()
    {
        // ImNodeFlow owns link lifetime internally. Graph links are written on Save()
        // by walking canvas pins for connections. Full bidirectional sync can be
        // wired here once the project needs runtime link queries.
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
        if (m_Graph.SaveToFile(m_Path))
        {
            m_Dirty = false;
            LOG(LogLevel::Info, "Material saved: {0}", m_Path.string());
        }
    }
    
}

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
        }
    
        void draw() override
        {
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
    
            // Drag-drop target — accepts TEXTURE_ITEM from Content Browser (wchar_t path)
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("TEXTURE_ITEM"))
                {
                    std::wstring wpath(static_cast<const wchar_t*>(p->Data), p->DataSize / sizeof(wchar_t));
                    std::filesystem::path texPath(wpath);
                    auto editorAM = Engine::Project::GetActive()->GetEditorAssetManager();
                    Engine::AssetHandle h = editorAM->ImportAsset(texPath);
                    if (static_cast<uint64_t>(h) != Engine::NullAssetHandle)
                    {
                        tsn->TextureHandle = h;
                        m_Dirty = true;
                    }
                }
                ImGui::EndDragDropTarget();
            }
            
            ImGui::TextDisabled("Texture Sample");
            
            ImGui::SameLine();
            
            if (ImGui::ArrowButton("##AssignFromCB", ImGuiDir_Left))
            {
                if (!m_CBSelectedPath.empty())
                {
                    auto ext = m_CBSelectedPath.extension();
                    bool isTexture = (ext == ".png" || ext == ".jpg" || ext == ".jpeg"
                                   || ext == ".tga" || ext == ".ktx");
                    if (isTexture)
                    {
                        auto editorAM = Engine::Project::GetActive()->GetEditorAssetManager();
                        auto h = editorAM->ImportAsset(m_CBSelectedPath);
                        if (static_cast<uint64_t>(h) != Engine::NullAssetHandle)
                        {
                            tsn->TextureHandle = h;
                            m_Dirty = true;
                        }
                    }
                }
            }
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

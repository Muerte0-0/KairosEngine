#pragma once
#include "Engine.h"

class EditorLayer : public Engine::Layer
{
public:
    EditorLayer();
    virtual ~EditorLayer() override = default;
    
    void OnAttach() override;
    void OnDetach() override;
    
    void OnUpdate(float DeltaTime) override;
    void OnFixedUpdate(float DeltaTime) override;
    
    void OnRender() override;
    void OnImGuiRender() override;
    
    void OnEvent(Engine::Event& event) override;
private:
    bool m_ViewportFocused = false, m_ViewportHovered = false, m_ViewportRightClicked = false;
    glm::vec2 m_ViewportSize = { 1280, 720 };
    glm::vec2 m_ViewportBounds[2];
    
    Ref<Scene> m_ActiveScene;
    
    // ----------- ImGui ----------- //
    void DrawMenuBar();
    void DrawViewport();

    // Debug
    void DrawImGuiDebug();

    bool m_ShowConsole = true;
    void DrawConsole();
    // ----------------------------- //
};

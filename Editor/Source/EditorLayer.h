#pragma once
#include "Engine/Core/Layer.h"

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
};

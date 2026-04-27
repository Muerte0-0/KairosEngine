#pragma once
#include "Engine/Core/Layer.h"

namespace Engine
{
	class ImGuiLayer : public Layer
	{
	public:
		~ImGuiLayer() override = default;
        
		virtual void Begin(float deltaTime) = 0;
		virtual void End() = 0;
        
		static Scope<ImGuiLayer> Create();
	};
}

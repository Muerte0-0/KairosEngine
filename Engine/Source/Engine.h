#pragma once

// -------- Application ---------- //
#include "Engine/Core/Application.h"
#include "Engine/Core/Layer.h"

#include "Engine/Debugging/Log.h"
#include "Engine/Debugging/Assert.h"
// ------------------------------- //

// ------------ Input ------------ //

// ------------------------------- //

// -------------- GUI ------------ //
#include "imgui.h"
#include "Engine/ImGui/ImGuiLayer.h"
#include "Engine/ImGui/ImGuiLogSink.h"
// ------------------------------- //

// ------------- Scene ------------- //
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Components.h"
#include "Engine/Scene/Entity.h"
// ------------------------------- //

// ---------- Renderer ----------- //
#include "Engine/Renderer/Renderer.h"
// ------------------------------- //

// ------------ Utils ------------ //

// ------------------------------- //

using namespace std;
using namespace Engine;
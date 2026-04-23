#pragma once

#include "Engine/Debugging/Log.h"
#include "Engine/Debugging/Assert.h"

// -------- Application ---------- //
#include "Engine/Core/Application.h"
#include "Engine/Core/Layer.h"

#include "Engine/Math/Math.h"
// ------------------------------- //

// ------------ Input ------------ //
#include "Engine/Input/Input.h"
// ------------------------------- //

// -------------- GUI ------------ //
#include "imgui.h"
#include "Engine/ImGui/ImGuiLayer.h"
#include "Engine/ImGui/ImGuiLogSink.h"
// ------------------------------- //

// ------------- Asset Registry ------------------- //
#include "Engine/Assets/Editor/AssetImporter.h"
#include "Engine/Assets/Editor/EditorAssetManager.h"
#include "Engine/Assets/AssetManager.h"
// ------------------------------------------------ //

// ------------- Scene ------------- //
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Components.h"
#include "Engine/Scene/Entity.h"

#include "Engine/Project/Project.h"
// --------------------------------- //

// ---------- Renderer ----------- //
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/SceneRenderer.h"

#include "Engine/Renderer/RHI/Buffer.h"

// Resources
#include "Engine/Renderer/RHI/Resources/Mesh.h"
#include "Engine/Renderer/RHI/Resources/Texture.h"
// ------------------------------- //

// ---------- Factories ----------- //
#include "Engine/Renderer/RHI/Factories/ModelFactory.h"
// ------------------------------- //

// ---------- Cameras ----------- //
#include "Engine/Renderer/Cameras/SceneCamera.h"
#include "Engine/Renderer/Cameras/SceneCameraController.h"
#include "Engine/Renderer/Cameras/CameraManager.h"
// ------------------------------- //

// ---------- Materials ---------- //
#include "Engine/Materials/MaterialGraph.h"
#include "Engine/Materials/MaterialGraphCompiler.h"
// ------------------------------- //

// ------------ Utils ------------ //
#include "Engine/Utils/PlatformUtils.h"
// ------------------------------- //

using namespace std;
using namespace Engine;

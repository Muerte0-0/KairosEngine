#include "kepch.h"
#include "Material.h"

#include "Engine/Renderer/RHI/RenderAPI.h"
#include "Engine/Renderer/Renderer.h"

namespace Engine
{
    Ref<Material> Material::Create()
    {
        // Forward to the active API factory — implemented in VulkanMaterial.cpp
        // via RenderAPI::CreateMaterial().
        RenderAPI* api = Renderer::GetAPI();
        ASSERT(api, "Material::Create: no active RenderAPI.");
        return api->CreateMaterial();
    }

    Ref<Material> Material::GetDefault()
    {
        static Ref<Material> s_Default;
        if (!s_Default)
        {
            s_Default = Material::Create();
            s_Default->SetName("Default");
            // Params already default to white albedo, 0 metallic, 1 roughness
        }
        return s_Default;
    }

} // namespace Engine

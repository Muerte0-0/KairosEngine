#include "kepch.h"
#include "Material.h"

#include "Engine/Renderer/RHI/RenderAPI.h"
#include "Engine/Renderer/Renderer.h"

namespace Engine
{
    // File-scope so ResetDefault() can null it without touching the static local.
    static Ref<Material> s_DefaultMaterial;

    Ref<Material> Material::Create()
    {
        RenderAPI* api = Renderer::GetAPI();
        ASSERT(api, "Material::Create: no active RenderAPI.");
        return api->CreateMaterial();
    }

    Ref<Material> Material::GetDefault()
    {
        if (!s_DefaultMaterial)
        {
            s_DefaultMaterial = Material::Create();
            s_DefaultMaterial->SetName("Default");
            // Params already default to white albedo, 0 metallic, 1 roughness
        }
        return s_DefaultMaterial;
    }

    void Material::ResetDefault()
    {
        s_DefaultMaterial.reset();
    }

} // namespace Engine

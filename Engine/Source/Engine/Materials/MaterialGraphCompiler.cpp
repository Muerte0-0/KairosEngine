#include "kepch.h"
#include "MaterialGraphCompiler.h"

#include "Engine/Assets/AssetManager.h"
#include "Engine/Debugging/Log.h"
#include "Engine/Renderer/RHI/Resources/Texture.h"

namespace Engine
{

// -----------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------

// Walk Links backward from an input pin to find the node feeding into it.
MaterialNode* MaterialGraphCompiler::ResolveSourceNode(const MaterialGraph& graph, uint32_t inputPinID)
{
    for (const MaterialLink& link : graph.Links)
    {
        if (link.ToPin != inputPinID)
            continue;

        // link.FromPin is an output pin — find which node owns it
        for (const auto& node : graph.Nodes)
        {
            for (const MaterialPin& out : node->Outputs)
            {
                if (out.ID == link.FromPin)
                    return node.get();
            }
        }
    }
    return nullptr;
}

Ref<Texture> MaterialGraphCompiler::LoadTextureForNode(const TextureSampleNode& node)
{
    if (static_cast<uint64_t>(node.TextureHandle) == NullAssetHandle)
        return nullptr;

    Ref<Texture> tex = AssetManager::GetAsset<Texture>(node.TextureHandle);
    if (!tex)
    {
        LOG(LogLevel::Warning,
            "MaterialGraphCompiler: TextureSampleNode references unresolvable handle {}.",
            static_cast<uint64_t>(node.TextureHandle));
    }
    return tex;
}

// -----------------------------------------------------------------------
// Compile
// -----------------------------------------------------------------------
Ref<Material> MaterialGraphCompiler::Compile(const MaterialGraph& graph)
{
    // Find PBROutputNode
    const PBROutputNode* outputNode = nullptr;
    for (const auto& node : graph.Nodes)
    {
        if (node->GetTypeName() == "PBROutput")
        {
            outputNode = static_cast<const PBROutputNode*>(node.get());
            break;
        }
    }

    if (!outputNode)
    {
        LOG(LogLevel::Warning, "MaterialGraphCompiler: graph '{}' has no PBROutputNode.", graph.Name);
        return nullptr;
    }

    Ref<Material> mat = Material::Create();
    mat->SetName(graph.Name);

    // PBROutputNode input pins by index (see PBROutputNode::Create):
    //   0 = BaseColor, 1 = Metallic, 2 = Roughness, 3 = Normal, 4 = Emissive, 5 = AmbientOcc
    struct PinSlot { size_t index; const char* name; };
    constexpr std::array<PinSlot, 6> pinSlots = {{
        {0, "BaseColor"}, {1, "Metallic"}, {2, "Roughness"},
        {3, "Normal"},    {4, "Emissive"}, {5, "AmbientOcc"}
    }};

    for (size_t i = 0; i < outputNode->Inputs.size(); ++i)
    {
        const MaterialPin& inputPin = outputNode->Inputs[i];
        MaterialNode* srcNode = ResolveSourceNode(graph, inputPin.ID);

        if (!srcNode)
            continue; // unconnected — fallback textures handle it

        const std::string& srcType = srcNode->GetTypeName();

        if (srcType == "TextureSample")
        {
            auto* texNode = static_cast<TextureSampleNode*>(srcNode);
            Ref<Texture> tex = LoadTextureForNode(*texNode);

            switch (i)
            {
                case 0: mat->Albedo            = tex; break; // BaseColor
                case 3: mat->Normal            = tex; break; // Normal
                case 4: mat->Emissive          = tex; break; // Emissive
                case 5: mat->AO                = tex; break; // AmbientOcc
                // Metallic/Roughness from texture not yet split — assign to MetallicRoughness slot
                case 1:
                case 2: mat->MetallicRoughness = tex; break;
                default: break;
            }
        }
        else if (srcType == "ConstantVec3")
        {
            auto* constNode = static_cast<ConstantVec3Node*>(srcNode);
            switch (i)
            {
                case 0: // BaseColor
                    mat->Params.AlbedoColor = glm::vec4(constNode->Value, 1.0f);
                    break;
                case 1: // Metallic
                    mat->Params.Metallic = constNode->Value.x;
                    break;
                case 2: // Roughness
                    mat->Params.Roughness = constNode->Value.x;
                    break;
                default: break;
            }
        }
        // Multiply / Lerp nodes not yet evaluated — treated as unconnected
    }

    return mat;
}

} // namespace Engine

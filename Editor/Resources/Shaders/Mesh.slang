// Mesh.slang — KairosEngine standard mesh shader
//
// Vertex layout matches Engine::Vertex in RendererUtils.h exactly:
//   float3 Position, float3 Normal, float3 Tangent, float3 Bitangent, float2 TexCoord
//
// Set 0, Binding 0 — FrameData UBO (View, Projection) — one per frame
// Push constant     — PushConstantObject (Model matrix) — one per draw call
//
// NOTE: Material textures (set=1) are intentionally absent until the
// descriptor-set wiring in the Material system is implemented.
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// Structs
// -----------------------------------------------------------------------

struct VSInput
{
    float3 Position  : POSITION;
    float3 Normal    : NORMAL;
    float3 Tangent   : TANGENT;
    float3 Bitangent : BITANGENT;
    float2 TexCoord  : TEXCOORD0;
};

struct VSOutput
{
    float4 ClipPosition : SV_Position;
    float3 WorldPos     : TEXCOORD0;
    float3 Normal       : TEXCOORD1;
    float2 TexCoord     : TEXCOORD2;
};

// -----------------------------------------------------------------------
// Push constant — per draw call (matches C++ PushConstantObject)
// -----------------------------------------------------------------------
struct PushConstantObject
{
    float4x4 Model;
};

[[vk::push_constant]]
PushConstantObject u_PushConstants;

// -----------------------------------------------------------------------
// Uniform buffer — per frame (set=0, binding=0)
// Matches C++ UniformBufferObject: View, Proj
// -----------------------------------------------------------------------
struct FrameData
{
    float4x4 View;
    float4x4 Projection;
};

[[vk::binding(0, 0)]]
ConstantBuffer<FrameData> u_FrameData;

// -----------------------------------------------------------------------
// Vertex Shader
// -----------------------------------------------------------------------
[shader("vertex")]
VSOutput vertMain(VSInput input)
{
    VSOutput output;

    float4 worldPosition    = mul(u_PushConstants.Model, float4(input.Position, 1.0));
    output.ClipPosition     = mul(u_FrameData.Projection, mul(u_FrameData.View, worldPosition));
    output.WorldPos         = worldPosition.xyz;

    // Normal matrix — upper-left 3x3 of Model (valid for uniform scale)
    float3x3 normalMatrix   = (float3x3)u_PushConstants.Model;
    output.Normal           = normalize(mul(normalMatrix, input.Normal));
    output.TexCoord         = input.TexCoord;

    return output;
}

// -----------------------------------------------------------------------
// Fragment Shader — simple Lambertian shading from vertex normals.
// No texture sampling until Material system descriptor sets are wired up.
// -----------------------------------------------------------------------
[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target
{
    // Hardcoded directional light — placeholder until lights are in the UBO
    float3 lightDir = normalize(float3(1.0, 2.0, 1.0));
    float3 normal   = normalize(input.Normal);

    float  NdotL    = max(dot(normal, lightDir), 0.0);
    float3 ambient  = float3(0.08, 0.08, 0.10);
    float3 diffuse  = float3(0.85, 0.85, 0.85) * NdotL;

    return float4(ambient + diffuse, 1.0);
}

// Mesh.slang — KairosEngine standard mesh shader
//
// Vertex layout matches Engine::Vertex in RendererUtils.h exactly:
//   float3 Position, float3 Normal, float3 Tangent, float3 Bitangent, float2 TexCoord
//
// Set 0, Binding 0 — FrameData UBO (View, Projection) — one per frame
// Set 1, Bindings 0-4 — Material textures (albedo, normal, metallicRough, ao, emissive)
// Set 1, Binding 5    — MaterialParams UBO (albedoColor, metallic, roughness, emissiveScale)
// Push constant       — PushConstantObject (Model matrix) — one per draw call
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

// set 1 — material
[[vk::binding(0, 1)]] Sampler2D albedoTex;
[[vk::binding(1, 1)]] Sampler2D normalTex;
[[vk::binding(2, 1)]] Sampler2D metallicRoughTex;
[[vk::binding(3, 1)]] Sampler2D aoTex;
[[vk::binding(4, 1)]] Sampler2D emissiveTex;

[[vk::binding(5, 1)]] cbuffer MaterialParams
{
    float4 albedoColor;
    float  metallic;
    float  roughness;
    float  emissiveScale;
    float  _pad;
};

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
// Fragment Shader — albedo + AO + emissive + Lambertian diffuse.
// Normal map decoded but TBN deferred to PBR pass (needs tangent in VSOutput).
// -----------------------------------------------------------------------
[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target
{
    // ---- Albedo ----------------------------------------------------------
    float4 albedoSample = albedoTex.Sample(input.TexCoord);
    float3 baseColor    = albedoSample.rgb * albedoColor.rgb;

    // ---- AO --------------------------------------------------------------
    float ao = aoTex.Sample(input.TexCoord).r;

    // ---- Emissive --------------------------------------------------------
    float3 emissive = emissiveTex.Sample(input.TexCoord).rgb * emissiveScale;

    // ---- Lambertian light ------------------------------------------------
    float3 N        = normalize(input.Normal);
    float3 lightDir = normalize(float3(1.0, 2.0, 1.0));
    float  NdotL    = max(dot(N, lightDir), 0.0);

    float3 ambient = float3(0.08, 0.08, 0.10) * baseColor * ao;
    float3 diffuse = baseColor * float3(0.85, 0.85, 0.85) * NdotL;

    float3 color = ambient + diffuse + emissive;
    return float4(color, albedoSample.a);
}

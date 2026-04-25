struct VSInput
{
    float3 Position  : POSITION;
    float3 Normal    : NORMAL;
    float3 Tangent   : TANGENT;
    float3 Bitangent : BITANGENT;
    float2 TexCoord  : TEXCOORD0;
};

struct PushConstantObject
{
    float4x4 Model;
};
[[vk::push_constant]] PushConstantObject u_PushConstants;

struct SceneData
{
    float4x4 View;
    float4x4 Projection;
    float4x4 LightViewProj;
};
[[vk::binding(0, 0)]] ConstantBuffer<SceneData> u_SceneData;

[shader("vertex")]
float4 vertMain(VSInput input) : SV_Position
{
    float4 worldPos = mul(u_PushConstants.Model, float4(input.Position, 1.0f));
    return mul(u_SceneData.LightViewProj, worldPos);
}

struct VSInput
{
	float3 inPosition;
	float3 inColor;
};

struct UniformBuffer
{
	float4x4 View;
	float4x4 Proj;
};
ConstantBuffer<UniformBuffer> ubo;

struct PushConstantBuffer
{
	float4x4 Model;
};
[[vk::push_constant]]
ConstantBuffer<PushConstantBuffer> pushConstants;

struct VSOutput
{
	float4 pos : SV_Position;
	float3 color : COLOR0;
};

[shader("vertex")]
VSOutput vertMain(VSInput input)
{
	VSOutput output;
	output.pos = mul(ubo.Proj, mul(ubo.View, mul(pushConstants.Model, float4(input.inPosition, 1.0))));
	output.color = input.inColor;
	return output;
}

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_TARGET
{
	return float4(input.color, 1.0);
}

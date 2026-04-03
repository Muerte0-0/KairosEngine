struct VSInput
{
	float3 inPosition;
	float3 inColor;
};

struct VSOutput
{
	float4 pos : SV_Position;
	float3 color : COLOR0;
};

[shader("vertex")]
VSOutput vertMain(VSInput input)
{
	VSOutput output;
	output.pos = float4(input.inPosition, 1.0);
	output.color = input.inColor;
	return output;
}

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_TARGET
{
	return float4(input.color, 1.0);
}

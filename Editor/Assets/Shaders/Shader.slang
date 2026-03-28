struct VSOutput
{
	float4 pos : SV_Position;
	float3 color : COLOR0;
};

[shader("vertex")]
VSOutput vertMain(uint vertexID : SV_VertexID)
{
	static const float2 positions[3] =
	{
		float2(0.0, -0.55),
		float2(0.55, 0.55),
		float2(-0.55, 0.55)
	};

	static const float3 colors[3] =
	{
		float3(1.0, 0.2, 0.2),
		float3(0.2, 1.0, 0.2),
		float3(0.2, 0.4, 1.0)
	};

	VSOutput output;
	output.pos = float4(positions[vertexID], 0.0, 1.0);
	output.color = colors[vertexID];
	return output;
}

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_TARGET
{
	return float4(input.color, 1.0);
}

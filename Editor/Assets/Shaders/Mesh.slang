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
		float2(0.0, -0.5),
		float2(0.5, 0.5),
		float2(-0.5, 0.5)
	};

	static const float3 colors[3] =
	{
		float3(1.0, 0.2, 0.2),
		float3(0.2, 1.0, 0.2),
		float3(0.2, 0.4, 1.0)
	};

	VSOutput output;
	float2 pos = positions[vertexID];
    pos.y *= -1.0;
	output.pos = float4(pos, 0.0, 1.0);
	output.color = colors[vertexID];
	return output;
}

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_TARGET
{
	return float4(input.color, 1.0);
}

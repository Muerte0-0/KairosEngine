#type vertex
#version 450

// vulkan NDC:	x: -1(left), 1(right)
//				y: -1(top), 1(bottom)

vec2 positions[3] = vec2[](
	vec2(0.0, -0.25),
	vec2(0.25, 0.25),
	vec2(-0.25, 0.25)
);

vec3 colors[3] = vec3[](
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0)
);

layout(location = 0) out vec3 fragColor;

void main() {
	//gl_Position = vec4(positions[gl_VertexID], 0.1, 1.0);
	//fragColor = colors[gl_VertexID];
	gl_Position = vec4(positions[gl_VertexIndex], 0.1, 1.0);
	fragColor = colors[gl_VertexIndex];
}

#type fragment
#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(fragColor, 1.0);
}
#type vertex
#version 450

layout(binding = 0) uniform SceneDataBlock
{
	mat4 ViewProjectionMatrix;
	mat4 CameraTransform;
}u_SceneData;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;

layout(location = 0) out vec3 out_Normal;
layout(location = 1) out vec3 out_Color;

void main()
{
	gl_Position = u_SceneData.ViewProjectionMatrix
					//* u_SceneData.CameraTransform
					* vec4(a_Position, 1.0);

	out_Normal = normalize(mat3(u_SceneData.CameraTransform) * a_Normal);
	out_Color = a_Normal;
}

#type fragment
#version 450

layout(location = 0) in vec3 in_Normal;
layout(location = 1) in vec3 in_Color;

layout(location = 0) out vec4 out_Color;

void main()
{
	vec3 lightDir = normalize(vec3(0.5, 1, 0));
	float intensity = max(dot(in_Normal, lightDir), 0.0);
	
	out_Color = vec4(intensity + in_Color, 1.0) * 0.5 + 0.5;
}
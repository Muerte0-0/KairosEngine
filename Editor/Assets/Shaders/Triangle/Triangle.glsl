#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;


layout(location = 0) out vec3 v_Position;
layout(location = 1) out vec4 v_Color;

void main()
{
	v_Position = a_Position;
	v_Color = a_Color;

	gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 450

layout(location = 0) out vec4 color;

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec4 v_Color;

void main()
{
	//color = mix(vec4(v_Position * 0.5 + 0.5, 1.0), v_Color, 0.5);
	color = v_Color;
}
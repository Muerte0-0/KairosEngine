#type vertex
#version 450
#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec4 a_Position;
layout(location = 1) in vec4 a_Color;

layout(location = 0) out vec4 v_Color;

void main()
{
	gl_Position = a_Position;
	v_Color = a_Color;
}

#type fragment
#version 450

layout(location = 0) out vec4 color;

layout(location = 0) in vec4 v_Color;

void main()
{
	//color = mix(vec4(v_Position * 0.5 + 0.5), v_Color, 0.5);
	color = v_Color;
}
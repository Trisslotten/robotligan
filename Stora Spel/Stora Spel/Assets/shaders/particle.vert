#version 440 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec4 a_color;
layout(location = 2) in float a_size;
layout(location = 3) in float a_time;

layout(location = 0) out vec3 g_pos;
layout(location = 1) out vec4 g_color;
layout(location = 2) out float g_size;
layout(location = 3) out float g_time;

void main()
{
	g_pos = a_pos;
	g_color = a_color;
	g_size = a_size;
	g_time = a_time;
}
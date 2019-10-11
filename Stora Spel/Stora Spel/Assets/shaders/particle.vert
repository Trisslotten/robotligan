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
	//g_pos = vec3(0);
	g_color = a_color;
	//g_color = vec4(1.0);
	//g_color = vec4(.5,.5,0.5, 0.01);
	g_size = a_size;
	//g_size = 1.0;
	g_time = a_time;
	//g_time = 5.0;

	//g_pos.y = -2.0;
	//if (g_pos.x == 0.0 || g_pos.x == 1.0 || g_pos.x == 2.0)
	//	g_size = 1.0;
	//gl_Position = cam_transform * vec4(pos, 1.0);
	//gl_Position = vec4(pos, 1.0);
}
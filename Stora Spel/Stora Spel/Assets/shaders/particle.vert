#version 440 core

layout(location = 0) in vec3 a_pos;

layout(location = 0) out vec3 g_pos;

void main()
{
	g_pos = a_pos;
	//gl_Position = cam_transform * vec4(pos, 1.0);
	//gl_Position = vec4(pos, 1.0);
}
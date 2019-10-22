#version 440 core

layout(location = 0) in vec3 pos;

uniform mat4 cam_transform;

void main()
{
	gl_Position = cam_transform * vec4(pos, 1.0);
}
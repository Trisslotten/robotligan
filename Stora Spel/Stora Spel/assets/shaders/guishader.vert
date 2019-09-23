#version 440 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 tex;

out vec2 tex_coords;

uniform mat4 cam_transform;

void main()
{
	gl_Position = cam_transform * vec4(pos, 0, 1);
	tex_coords = tex;
}
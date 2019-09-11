#version 440 core

layout(location = 0) in vec3 pos;

out vec2 v_uv;

uniform mat4 cam_transform;

void main()
{
	v_uv = (pos.xy + 1.0)/4.0;
	gl_Position = cam_transform * vec4(pos, 1);
}
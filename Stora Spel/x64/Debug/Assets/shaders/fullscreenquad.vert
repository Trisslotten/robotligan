#version 440 core

layout(location = 0) in vec3 pos;

out vec2 v_uv;

void main()
{
	v_uv = (pos.xy + 1.0)/2.0;
	gl_Position = vec4(pos, 1);
}
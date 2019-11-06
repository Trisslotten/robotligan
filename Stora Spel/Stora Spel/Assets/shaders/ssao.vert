#version 440 core

layout(location = 0) in vec3 pos;

out vec2 v_uv;
out vec2 v_pos;

void main()
{
	v_uv = (pos.xy + 1.0)/2.0;
	v_pos = pos.xy;
	gl_Position = vec4(pos, 1);
}
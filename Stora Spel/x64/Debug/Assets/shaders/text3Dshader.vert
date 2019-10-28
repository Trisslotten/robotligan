#version 440 core
uniform vec3 offset;
uniform vec3 center;
uniform float size;
uniform uint int_size;

uniform mat4 rotation;
uniform mat4 cam_transform;
uniform int character;

layout(location = 0) in vec3 pos;
out vec2 v_tex;


void main()
{
	vec2 qpos = pos.xy;

	int jumps_down = character / 16;
    int jumps_right = character % 16;

	vec2 uv = vec2(jumps_right, jumps_down)/16;

	v_tex = uv + (qpos*0.97 + 1)/32;

	vec3 vpos = pos * size + offset;
	vpos = (rotation * vec4(vpos, 0)).xyz;
	vpos = vpos + center;
	vec4 out_pos = cam_transform * vec4(vpos,1);

	gl_Position = out_pos;
}
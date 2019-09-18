#version 440 core
uniform vec2 screen_dims;
uniform vec2 t_pos;
uniform uint size;

uniform int character;

layout(location = 0) in vec3 pos;
out vec2 v_tex;


void main()
{
	vec2 qpos = pos.xy;
	vec2 ble = t_pos/screen_dims;
	ble = ble*2 - 1;

	int jumps_down = character / 16;
    int jumps_right = character % 16;

	vec2 uv = vec2(jumps_right, jumps_down)/16;

	v_tex = uv + (qpos*0.97 + 1)/32;
	vec2 size_uv = vec2(size)/screen_dims;

	vec2 vpos = ble + qpos*size_uv;
	gl_Position = vec4(vpos,0,1);
}
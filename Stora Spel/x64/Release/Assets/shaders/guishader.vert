#version 440 core

layout(location = 0) in vec3 pos;

out vec2 tex_coords;

uniform vec2 screen_dims;
uniform vec2 texture_dims;
uniform vec2 t_pos;
uniform float t_scale;
uniform float t_scale_x;

void main()
{
	vec2 qpos = (pos.xy + 1);

	vec2 tex_ble = (texture_dims/screen_dims) * t_scale;
	tex_ble.x = (tex_ble.x / 100) * t_scale_x;

	vec2 ble = t_pos/screen_dims;
	ble = ble*2 - 1;

	vec2 vpos = ble + qpos * tex_ble;

	gl_Position = vec4(vpos, 0, 1);

	tex_coords = (pos.xy+1.0)/2.0;
	tex_coords.y = 1.0-tex_coords.y;
}
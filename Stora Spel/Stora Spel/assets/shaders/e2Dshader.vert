#version 440 core

layout(location = 0) in vec3 pos;

out vec2 tex_coords;

uniform vec2 texture_dims;
uniform vec2 t_pos;
uniform float t_scale;
uniform mat4 t_rot;
uniform mat4 cam_transform;

void main()
{
	vec3 vpos = (pos + t_pos) * t_scale;
	gl_Position = cam_transform * vec4(vpos, 1);

	tex_coords = (pos.xy+1.0)/2.0;
	tex_coords.y = 1.0-tex_coords.y;
}
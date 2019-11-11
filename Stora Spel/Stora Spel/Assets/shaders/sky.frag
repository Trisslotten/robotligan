#version 440 core

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_emission;

uniform sampler2D texture_sky;

in vec2 uv;
in vec4 proj_pos;

void main() {
	out_color = vec4(texture(texture_sky, uv).rgb, 1.);
	out_emission = vec4(pow(out_color.rgb, vec3(4.)), 0.0);
}
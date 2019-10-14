#version 440 core

out vec4 out_color;

in vec2 v_uv;

uniform sampler2D texture_color;
uniform sampler2D texture_emission;

void main() {
	vec3 color = texture(texture_color, v_uv).rgb;

	color += texture(texture_emission, v_uv, 1). rgb;

	out_color = vec4(color, 1);
}
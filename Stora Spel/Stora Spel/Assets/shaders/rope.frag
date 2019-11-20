#version 440 core

layout(location = 0) out vec4 out_color;

uniform sampler2D texture_diffuse;

in vec2 v_uv;

void main() {
	vec3 color = texture(texture_diffuse, v_uv).rgb;
	out_color = vec4(color, 1);
}
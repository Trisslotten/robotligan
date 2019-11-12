#version 440 core

out vec4 out_color;

in vec2 v_uv;

uniform sampler2D texture_color;
uniform sampler2D texture_emission;
uniform sampler2D texture_ssao;

void main() {
	vec3 color = texture(texture_color, v_uv).rgb;

	vec4 emission = texture(texture_emission, v_uv, 1);
	float ao = texture(texture_ssao, v_uv).r;
	color*=ao;
	color += emission.rgb*0.9;
	

	out_color = vec4(color, 1);
}
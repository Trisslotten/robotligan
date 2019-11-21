#version 440 core

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_emission;
layout(location = 2) out vec4 out_depth;

uniform sampler2D texture_diffuse;

in vec2 v_uv;
in vec3 v_normal;

void main() {
	vec3 color = texture(texture_diffuse, v_uv).rgb;
	out_color = vec4(color, 1);
	out_emission = vec4(0,0,0,1);
	out_depth = vec4(gl_FragCoord.z, 0,0,0);
}
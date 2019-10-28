#version 440 core

out vec4 out_color;
out vec4 out_emission;

uniform vec3 color;

void main() {
	out_color = vec4(color,1);
	out_emission = vec4(1,0,0,0);
}
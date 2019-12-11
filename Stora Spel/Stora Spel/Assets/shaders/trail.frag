#version 440 core

out vec4 out_color;
out vec4 out_emission;

uniform vec4 color;

void main() {
	out_color = vec4(0,0,0,0);
	out_emission = vec4(color.rgb, 0.2);
}
#version 440 core

out vec4 depths;

in vec3 frag_pos;

uniform vec3 shadow_light_pos;

void main() {
	float depth = length(frag_pos - shadow_light_pos);
	depths = vec4(depth, depth*depth, 0, 0);
}

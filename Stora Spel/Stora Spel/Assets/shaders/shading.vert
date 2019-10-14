#version 440 core

uniform mat4 shadow_transform;

out vec4 v_shadow_space;

void handleShading(vec3 position) {
	v_shadow_space = shadow_transform * vec4(position, 1.0);
}
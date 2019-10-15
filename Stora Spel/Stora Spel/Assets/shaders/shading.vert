#version 440 core

#define MAX_SHADOWS 4

uniform int num_shadows;
uniform mat4 shadow_transforms[MAX_SHADOWS];

out vec4 v_shadow_spaces[MAX_SHADOWS];

void handleShading(vec3 position) {
	for(int i = 0; i < num_shadows; i++) {
		v_shadow_spaces[i] = shadow_transforms[i] * vec4(position, 1.0);
	}
}
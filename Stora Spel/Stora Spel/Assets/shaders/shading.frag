#version 440 core

#define MAX_SHADOWS 4

#define MAX_LIGHTS 16

in vec4 v_shadow_spaces[MAX_SHADOWS];

uniform int num_shadows;
uniform sampler2D shadow_maps[MAX_SHADOWS];
uniform mat4 shadow_transforms[MAX_SHADOWS];
uniform vec3 shadow_light_positions[MAX_SHADOWS];

uniform vec3 light_pos[MAX_LIGHTS];
uniform vec3 light_col[MAX_LIGHTS];
uniform float light_radius[MAX_LIGHTS];
uniform float light_amb[MAX_LIGHTS];
uniform int NR_OF_LIGHTS;

// https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
float rand(vec2 n) { 
	n = mod(n, vec2(20000));
	return 2*fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453)-1;
}

vec3 dither() {
	vec3 result = vec3(0);
	float num_colors = 256.0;
	float val = 0.5*rand(gl_FragCoord.xy)/num_colors;
	result += val;
	return result;
}

float shadow(vec3 position, int index) {
	float result = 1.0;

	vec4 shadow_space = v_shadow_spaces[index];
	shadow_space /= shadow_space.w;

	vec2 uv = (shadow_space.xy+1.)*0.5;
	if(uv.x < 0. || uv.x > 1. || uv.y < 0. || uv.y > 1.) {
		return 1.0;
	}
	float frag_depth = length(position - shadow_light_positions[index]);

	vec2 depths = textureLod(shadow_maps[index], uv, 0).xy;
	float E_x2 = depths.y;
	float Ex_2 = depths.x*depths.x;
	float variance = E_x2 - Ex_2;

	float mD = depths.x - frag_depth;
	float p = variance / (variance + mD*mD);
	
	result = 0.0;
	if(frag_depth <= depths.x)
		result = 1.0;
	result = max(p, result);
	result = clamp(result, 0.0, 1.0);

	return result;
}

vec3 shading(vec3 position, vec3 normal) {

	vec3 lighting = vec3(0);
	for(int l = 0; l < NR_OF_LIGHTS; l++){
		vec3 pointToLight = light_pos[l] - position;
		vec3 light_dir = normalize(pointToLight);

		float intensity = 1.f - clamp(length(pointToLight), 0, light_radius[l]) / light_radius[l];
		vec3 diffuse = max(dot(light_dir, normal), 0) * light_col[l] * intensity;

		lighting += diffuse;
		lighting += light_amb[l];
	}

	for(int i = 0; i < num_shadows; i++) {
		vec4 shadow_space = v_shadow_spaces[i];
		shadow_space.xyz /= shadow_space.w;

		vec3 ld = normalize(shadow_light_positions[i] - position);
		if(shadow_space.w > 0) {
			vec3 spot_light = vec3(0.25);
			// diffuse
			spot_light *= max(dot(ld, normal), 0);
			// in spot light circle
			spot_light *= smoothstep( 1.0,  0.5, max(shadow_space.x,shadow_space.y));
			spot_light *= smoothstep(-1.0, -0.5, min(shadow_space.x,shadow_space.y));
			// occlusion
			spot_light *= shadow(position, i);

			lighting += spot_light;
		}
	}

	return lighting;
}

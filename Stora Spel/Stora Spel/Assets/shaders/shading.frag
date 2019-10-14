#version 440 core

in vec4 v_shadow_space;


uniform vec3 light_pos[64];
uniform vec3 light_col[64];
uniform float light_radius[64];
uniform float light_amb[64];
uniform int NR_OF_LIGHTS;

uniform sampler2D shadow_map;
uniform mat4 shadow_transform;
uniform vec3 shadow_light_pos;

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

float shadow(vec3 position) {
	float result = 1.0;

	vec4 shadow_space = v_shadow_space;
	shadow_space /= shadow_space.w;

	vec2 uv = (shadow_space.xy+1.)*0.5;
	if(uv.x < 0. || uv.x > 1. || uv.y < 0. || uv.y > 1.) {
		return 1.0;
	}
	float frag_depth = length(position - shadow_light_pos);

	vec2 depths = textureLod(shadow_map, uv, 1).xy;
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
	}

	for(int l = 0; l < NR_OF_LIGHTS; l++){
		lighting += light_amb[l];
	}

	vec4 shadow_space = shadow_transform * vec4(position, 1.0);
	shadow_space.xyz /= shadow_space.w;

	vec3 ld = normalize(shadow_light_pos - position);
	if(shadow_space.w > 0) {
		vec3 spot_light = vec3(0.5);
		// diffuse
		spot_light *= max(dot(ld, normal), 0);
		// in spot light circle
		spot_light *= smoothstep( 1.0,  0.5, max(shadow_space.x,shadow_space.y));
		spot_light *= smoothstep(-1.0, -0.5, min(shadow_space.x,shadow_space.y));
		// occlusion
		spot_light *= shadow(position);

		lighting += spot_light;
	}

	return lighting;
}

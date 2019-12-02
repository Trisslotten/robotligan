#version 440 core

#define MAX_SHADOWS 4

#define MAX_LIGHTS 32

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

in vec3 local_pos;
in vec3 local_normal;
in vec3 v_normal;

uniform sampler2D texture_roughness;
uniform float roughness_map_scale;
uniform int use_roughness;

uniform vec3 cam_position;

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
	float frag_depth = length(position - shadow_light_positions[index]);

	vec2 depths = texture(shadow_maps[index], uv).xy;
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


float triplanarRoughness() {
	float texture_scale = roughness_map_scale;
	float sharpness = 10.0;

	vec2 uv_x = local_pos.zy/texture_scale;
	vec2 uv_y = local_pos.xz/texture_scale;
	vec2 uv_z = local_pos.xy/texture_scale;

	float x = texture(texture_roughness, uv_x).r;
	float y = texture(texture_roughness, uv_y).r;
	float z = texture(texture_roughness, uv_z).r;

	vec3 weights = pow(abs(local_normal), sharpness.xxx);
	weights /= dot(weights, vec3(1));

	float result = 0;
	result += x * weights.x;
	result += y * weights.y;
	result += z * weights.z;

	return result;
}

float calcDiffuse(vec3 surf_pos, vec3 normal, vec3 light_dir) {
	float diffuse = max(dot(light_dir, normal), 0);
	return diffuse;
}
float calcSpecular(vec3 surf_pos, vec3 normal, vec3 light_dir, vec3 view_dir, float roughness) {
	float glossy = 1-roughness;
	vec3 half_vec = normalize(light_dir + view_dir);
	float specular = pow(clamp(dot(normal, half_vec), 0, 1), 1 + 1000.0 * glossy);
	return specular;
}

struct Lighting {
	vec3 specular;
	vec3 diffuse;
	vec3 ambient;
};

Lighting shading(vec3 position, vec3 normal) {
	float roughness = 0.;
	if(use_roughness != 0)
	{
		roughness = triplanarRoughness();
	}

	vec3 view_dir = normalize(cam_position - position);

	Lighting lighting;
	lighting.ambient = vec3(0);
	lighting.diffuse = vec3(0);
	lighting.specular = vec3(0);

	for(int l = 0; l < NR_OF_LIGHTS; l++){
		vec3 pointToLight = light_pos[l] - position;
		float radius = light_radius[l];
		lighting.ambient += light_amb[l];
		if(length(pointToLight) <= radius) {
			vec3 light_dir = normalize(pointToLight);
			vec3 light_color = light_col[l];

			float intensity = 1.f - clamp(length(pointToLight)/radius, 0., 1.);
			float diffuse = calcDiffuse(position, normal, light_dir);
			float specular = calcSpecular(position, normal, light_dir, view_dir, roughness);

			lighting.diffuse += diffuse * intensity * light_color;
			lighting.specular += specular * intensity * light_color;
		}
	}

	for(int i = 0; i < num_shadows; i++) {
		vec4 shadow_space = v_shadow_spaces[i];
		shadow_space.xyz /= shadow_space.w;

		vec2 uv = (shadow_space.xy+1.)*0.5;
		if(shadow_space.w > 0 && !(uv.x < 0. || uv.x > 1. || uv.y < 0. || uv.y > 1.)) {
			vec3 ld = normalize(shadow_light_positions[i] - position);
			vec3 light_color = vec3(0.4);

			vec2 q = abs(shadow_space.xy) - vec2(0.5);
  			float len = length(max(q,0.0)) + min(max(q.x, q.y), 0.0);
			float mask = smoothstep(0.5, 0.0, len);

			mask *= shadow(position, i);

			float diffuse = calcDiffuse(position, normal, ld);
			float specular = calcSpecular(position, normal, ld, view_dir, roughness);

			light_color *= mask;

			lighting.diffuse += light_color * diffuse;
			lighting.specular += light_color * specular;
		}
	}

	return lighting;
}

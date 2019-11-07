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

in vec3 local_pos;
in vec3 local_normal;
in vec3 v_normal;

uniform sampler2D texture_normal;
uniform sampler2D texture_metallic;
uniform float metallic_map_scale;
uniform float normal_map_scale;

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


vec3 normalMap(vec2 uv) {
	vec3 normal_tex;
	normal_tex.xy = texture(texture_normal, uv).rg * 2.0 - 1.0;
	normal_tex.z = sqrt(1.-normal_tex.x*normal_tex.x-normal_tex.y*normal_tex.y);
	return normalize(normal_tex);
}

vec3 calculateTangent(vec3 n) {
	vec3 tangent = cross(n, vec3(1,0,0));
	if(length(tangent) < 0.1) {
		tangent = cross(n, -vec3(0,1,0));
	}
	return normalize(tangent);
	//return n.yxz;
}
// https://medium.com/@bgolus/normal-mapping-for-a-triplanar-shader-10bf39dca05a
vec3 triplanarNormal() {
	float texture_scale = normal_map_scale;
	float sharpness = 10.0;

	vec2 uv_x = local_pos.zy/texture_scale;
	vec2 uv_y = local_pos.xz/texture_scale;
	vec2 uv_z = local_pos.xy/texture_scale;

	vec3 normal_x = normalMap(uv_x);
	vec3 normal_y = normalMap(uv_y);
	vec3 normal_z = normalMap(uv_z);

	vec3 vert_normal = normalize(v_normal);
	vec3 axis_sign = sign(vert_normal);

	vec3 tangent = calculateTangent(vert_normal);

	mat3 tbn = mat3(
		tangent,
		normalize(cross(tangent, vert_normal)), 
		vert_normal
	);

	vec3 weights = pow(abs(local_normal), sharpness.xxx);
	weights /= dot(weights, vec3(1));

	vec3 result = vec3(0);
	result += normal_x * weights.x * vec3(axis_sign.z,1,1);
	result += normal_y * weights.y * vec3(axis_sign.x,1,1);
	result += normal_z * weights.z * vec3(axis_sign.y,1,1);
	
	result = normalize(tbn * result);

	return result;
}


float calcDiffuse(vec3 surf_pos, vec3 normal, vec3 light_dir) {
	float diffuse = max(dot(light_dir, normal), 0);
	return diffuse;
}
float calcSpecular(vec3 surf_pos, vec3 normal, vec3 light_dir, vec3 view_dir) {
	vec3 half_vec = normalize(light_dir + view_dir);
	float specular = pow(clamp(dot(normal, half_vec), 0, 1), 100.0);
	return specular;
}

struct Lighting {
	vec3 specular;
	vec3 diffuse;
	vec3 ambient;
};

Lighting shading(vec3 position, float metallic) {
	vec3 normal = triplanarNormal();

	vec3 view_dir = normalize(cam_position - position);

	Lighting lighting;
	lighting.ambient = vec3(0);
	lighting.diffuse = vec3(0);
	lighting.specular = vec3(0);

	for(int l = 0; l < NR_OF_LIGHTS; l++){
		vec3 pointToLight = light_pos[l] - position;
		vec3 light_dir = normalize(pointToLight);
		vec3 light_color = light_col[l];

		float intensity = 1.f - clamp(length(pointToLight), 0, light_radius[l]) / light_radius[l];
		float diffuse = calcDiffuse(position, normal, light_dir);
		float specular = metallic * calcSpecular(position, normal, light_dir, view_dir);

		lighting.diffuse += diffuse * intensity * light_color;
		lighting.specular += specular * intensity * light_color;
		lighting.ambient += light_amb[l];
	}

	for(int i = 0; i < num_shadows; i++) {
		vec4 shadow_space = v_shadow_spaces[i];
		shadow_space.xyz /= shadow_space.w;

		vec3 ld = normalize(shadow_light_positions[i] - position);
		if(shadow_space.w > 0) {
			vec3 light_color = vec3(0.23);

			vec2 q = abs(shadow_space.xy) - vec2(0.5);
  			float len = length(max(q,0.0)) + min(max(q.x, q.y), 0.0);
			float mask = smoothstep(0.5, 0.0, len);

			mask *= shadow(position, i);

			float diffuse = calcDiffuse(position, normal, ld);
			float specular = metallic * calcSpecular(position, normal, ld, view_dir);

			light_color *= mask;

			lighting.diffuse += light_color * diffuse;
			lighting.specular += light_color * specular;
		}
	}

	return lighting;
}

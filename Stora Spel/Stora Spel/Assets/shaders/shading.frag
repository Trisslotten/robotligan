#version 440 core

#define MAX_SHADOWS 4

#define MAX_LIGHTS 32

#define MAX_PLANES 9

in vec4 v_shadow_spaces[MAX_SHADOWS];

uniform int num_shadows;
uniform sampler2D shadow_maps[MAX_SHADOWS];
uniform mat4 shadow_transforms[MAX_SHADOWS];
uniform vec3 shadow_light_positions[MAX_SHADOWS];

uniform vec4 light_pos_radius[MAX_LIGHTS];
uniform vec4 light_col_amb[MAX_LIGHTS];
uniform float light_sphere_radii[MAX_LIGHTS];
uniform int NR_OF_LIGHTS;

uniform vec3 plane_normals[MAX_PLANES];
uniform vec3 plane_positions[MAX_PLANES];
uniform vec3 plane_colors[MAX_PLANES];
uniform vec2 plane_sizes[MAX_PLANES];
uniform int plane_diffuse[MAX_PLANES];
uniform mat4 plane_matrices[2*MAX_PLANES];
uniform int NR_OF_PLANES;

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
	const float num_colors = 256.0;
	return vec3(0.5*rand(gl_FragCoord.xy)/num_colors);
}

const float near = 0.1;
const float far = 300.0;
float linearDepth(float depth)
{
	return far*near/(depth * (near - far) + far);
}

float shadow(vec3 shadow_space, int index) {
	vec2 uv = (shadow_space.xy+1.)*0.5;
	shadow_space.z = linearDepth(shadow_space.z*0.5 + 0.5);// length(position - shadow_light_positions[index]);

	vec2 depths = texture(shadow_maps[index], uv).xy;
	float variance = depths.y - depths.x*depths.x;
	float mD = depths.x - shadow_space.z;
	float p = variance / (variance + mD*mD);
	
	// lessThanEqual
	float result = float(shadow_space.z <= depths.x);
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

float calcDiffuse(const vec3 surf_pos, const vec3 normal, const vec3 light_dir) {
	float diffuse = max(dot(light_dir, normal), 0);
	return diffuse;
}
float calcSpecular(const vec3 surf_pos, const vec3 normal, const vec3 light_dir, const vec3 view_dir, const float roughness) {
	float glossy = 1-roughness;
	vec3 half_vec = normalize(light_dir + view_dir);
	float specular = pow(clamp(dot(normal, half_vec), 0, 1), 1 + 1000.0 * glossy);
	return specular;
}


vec3 calcSphereLightVec(const vec3 view_dir, const vec3 normal, const vec3 light_vec, const float radius)
{
	vec3 r = normalize(reflect(normalize(view_dir), normal));
	vec3 center_to_ray = dot(light_vec, r) * r - light_vec;
	vec3 closest_point = light_vec + center_to_ray * clamp(radius/length(center_to_ray),0,1);
	return normalize(closest_point);
}

// https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float sdBox( vec3 p, vec3 b )
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

struct Lighting {
	vec3 specular;
	vec3 diffuse;
	vec3 ambient;
};

Lighting shading(const vec3 position, const vec3 normal, const float reflective) {
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
		const vec4 pos_rad = light_pos_radius[l];
		vec3 pointToLight = pos_rad.xyz - position;
		const vec4 col_amb = light_col_amb[l];
		lighting.ambient += col_amb.w;

		vec3 light_dir = normalize(pointToLight);

		//float intensity = 1.f - clamp(length(pointToLight)/pos_rad.w, 0., 1.);
		float x2 = dot(pointToLight, pointToLight);
		float a = 254.5 / (pos_rad.w * pos_rad.w);
		float intensity = 1./(1.+ a * x2);
		if(intensity > 0.) {
			float diffuse = calcDiffuse(position, normal, light_dir);
			lighting.diffuse += diffuse * intensity * col_amb.rgb;

			if(reflective >= 1.0/255.0) {
				const float sphere_radius = light_sphere_radii[l];
				if(sphere_radius > 0.) {
					light_dir = calcSphereLightVec(view_dir, normal, pointToLight, sphere_radius);
				}
				float specular = calcSpecular(position, normal, light_dir, view_dir, roughness);
				lighting.specular += specular * intensity * col_amb.rgb;
			}
		}
	}
	
	//if(reflective >= 1.0/255.0) {
	vec3 r = normalize(reflect(normalize(view_dir), normal));
	for(int i = 0; i < NR_OF_PLANES; i++) {
		const vec2 sizes = plane_sizes[i];
		const vec3 ppos = plane_positions[i];
		const vec3 plane_normal = plane_normals[i];
		const vec3 tangent = vec3(0,1,0);
		const vec3 bitangent = normalize(cross(plane_normal, tangent));

		vec3 l = ppos - position;
		float dist2 = dot(l,l);

		float intensity = 1./(1.+ 0.1*dist2);

		const float t = dot(ppos - position, plane_normal) / dot(r, plane_normal);
		const vec3 v = (position + t * r) - ppos;
		if (abs(dot(v, tangent)) < sizes.x && abs(dot(v, bitangent)) < sizes.y && t < 0) {
			lighting.specular += plane_colors[i];
		}

		if (plane_diffuse[i] != 0) {
			float d = dot((position - ppos), plane_normal);
			int offset = int(d > 0.0);
			vec4 plane_space = plane_matrices[2*i+offset] * vec4(position, 1.0);
			plane_space.xyz /= plane_space.w;
			vec4 plane_space_norm = plane_matrices[2*i+offset] * vec4(plane_normal, 0.0);
			plane_space_norm.xyz /= plane_space_norm.w;
			float i2 = clamp(plane_space_norm.z, 0.f, 1.f);
			vec2 q = abs(plane_space.xy) - vec2(0.5);
  			float len = length(max(q,0.0)) + min(max(q.x, q.y), 0.0);
			float mask = smoothstep(0.8, 0.0, len);
			lighting.diffuse += 5.0 * i2 * mask * plane_colors[i] * intensity;
		}
	}
	//}

	

	//float dist = sdBox(position - vec3(10,0,0), vec3(0.1, 5, 5));
	//lighting.diffuse += vec3(1,0.5,0.5)/(1.+ 0.01 * dist*dist);
	
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

			mask *= shadow(shadow_space.xyz, i);

			float diffuse = calcDiffuse(position, normal, ld);
			float specular = calcSpecular(position, normal, ld, view_dir, roughness);

			light_color *= mask;

			lighting.diffuse += light_color * diffuse;
			lighting.specular += light_color * specular;
		}
	}

	return lighting;
}

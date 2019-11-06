#version 440 core

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_emission;

in vec3 local_pos;
in vec3 local_normal;
in vec3 frag_pos;
in vec2 v_tex;
in vec3 v_normal;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform sampler2D texture_emissive;
uniform sampler2D texture_normal;

uniform float material_scale;
uniform int num_diffuse_textures;
uniform int diffuse_index;

// from file "shading.frag"
vec3 shading(vec3 position, vec3 normal);
vec3 dither();

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
	float texture_scale = 5.0;
	float sharpness = 2.0;

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

void main()
{
	vec3 normal = triplanarNormal();
	
	vec2 tex = v_tex;
	float mat_dist = 1.0/float(num_diffuse_textures);
	float mat_offset = mat_dist * float(diffuse_index);
	tex.x = tex.x * mat_dist + mat_offset;

	vec4 surface_color = texture(texture_diffuse, tex);
	float alpha = surface_color.a;

	float emission_strength = texture(texture_emissive, v_tex).r;
	vec3 shading = shading(frag_pos, normal);
	vec3 color = surface_color.rgb * mix(shading, vec3(1), emission_strength);

	color += dither();

	out_color = vec4(color, 1);
	out_emission = vec4(emission_strength * surface_color.rgb, 1);
}
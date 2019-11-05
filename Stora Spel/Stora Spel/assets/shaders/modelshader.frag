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

// from file "shading.frag"
vec3 shading(vec3 position, vec3 normal);
vec3 dither();

vec3 normalMap(vec2 uv) {
	vec3 normal_tex;
	normal_tex.xy = texture(texture_normal, uv).rg * 2.0 - 1.0;
	normal_tex.z = sqrt(1.-normal_tex.x*normal_tex.x-normal_tex.y*normal_tex.y);
	return normalize(normal_tex);
}

// https://medium.com/@bgolus/normal-mapping-for-a-triplanar-shader-10bf39dca05a
vec3 triplanarNormal() {
	float texture_scale = 0.1;
	float sharpness = 2.0;

	vec2 uv_x = local_pos.zy/texture_scale;
	vec2 uv_y = local_pos.xz/texture_scale;
	vec2 uv_z = local_pos.xy/texture_scale;

	vec3 normal_x = normalMap(uv_x);
	vec3 normal_y = normalMap(uv_y);
	vec3 normal_z = normalMap(uv_z);
	/*
	vec3 axis_sign = sign(v_normal);

	normal_x.z *= axis_sign.x;
	normal_y.z *= axis_sign.y;
	normal_z.z *= axis_sign.z;
	*/
	vec3 vert_normal = normalize(v_normal);
	mat3 tbn = mat3(vert_normal.zyx, vert_normal.xzy,  vert_normal);

	vec3 weights = pow(abs(local_normal), sharpness.xxx);
	weights /= dot(weights, vec3(1));

	vec3 result = 
		normal_x * weights.x + 
		normal_y * weights.y + 
		normal_z * weights.z;
	
	result = normalize(tbn * normalize(result));

	return result;
}

void main()
{
	vec3 normal = triplanarNormal();
	vec3 surface_color = texture(texture_diffuse, v_tex).rgb;

	vec3 color = surface_color * shading(frag_pos, normal);	

	color += dither();

	out_color = vec4(color, 1);
	out_emission = vec4(0,0,0,1);
}
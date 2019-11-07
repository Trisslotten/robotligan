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

uniform int num_diffuse_textures;
uniform int diffuse_index;

uniform sampler2D texture_metallic;
uniform float metallic_map_scale;

struct Lighting {
	vec3 specular;
	vec3 diffuse;
	vec3 ambient;
};

// from file "shading.frag"
Lighting shading(vec3 position, float metallic);
vec3 dither();

float triplanarMetallic() {
	float texture_scale = metallic_map_scale;
	float sharpness = 1.0;

	vec2 uv_x = local_pos.zy/texture_scale;
	vec2 uv_y = local_pos.xz/texture_scale;
	vec2 uv_z = local_pos.xy/texture_scale;

	float x = texture(texture_metallic, uv_x).r;
	float y = texture(texture_metallic, uv_y).r;
	float z = texture(texture_metallic, uv_z).r;

	vec3 weights = pow(abs(local_normal), sharpness.xxx);
	weights /= dot(weights, vec3(1));

	float result = 0;
	result += x * weights.x;
	result += y * weights.y;
	result += z * weights.z;

	return result;
}

void main()
{
	float metallic = triplanarMetallic();

	// calculate diffuse texture coords
	vec2 tex = v_tex;
	float mat_dist = 1.0/float(num_diffuse_textures);
	float mat_offset = mat_dist * float(diffuse_index);
	tex.x = tex.x * mat_dist + mat_offset;

	float emission_strength = texture(texture_emissive, v_tex).r;

	vec4 surface_color = texture(texture_diffuse, tex);
	float alpha = surface_color.a;
	
	vec3 emission = emission_strength * surface_color.rgb;

	surface_color.rgb = mix(surface_color.rgb, 0.3333*dot(surface_color.rgb,vec3(1.0)).rrr, metallic);

	Lighting lighting = shading(frag_pos, metallic);
	vec3 shading = vec3(0);
	shading += lighting.ambient;
	shading += lighting.diffuse;
	shading += lighting.specular;
	shading *= surface_color;

	emission += 0.2*lighting.specular;

	vec3 color = surface_color.rgb;
	color *= mix(shading, vec3(1), emission_strength);
	color += dither();

	//float gamma = 2.2;
    //color = pow(color, vec3(gamma));

	//color = surface_color.rgb;

	out_color = vec4(color, alpha);
	out_emission = vec4(emission, 1);
}
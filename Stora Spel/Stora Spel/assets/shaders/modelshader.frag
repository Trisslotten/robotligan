#version 440 core

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_emission;
layout(location = 2) out vec4 out_normal;
layout(location = 3) out vec4 out_depth;
layout(location = 4) out vec4 out_pos;

in vec3 local_pos;
in vec3 local_normal;
in vec3 frag_pos;
in vec2 v_tex;
in vec3 v_normal;

uniform vec3 cam_position;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform sampler2D texture_emissive;
uniform int use_emissive;

uniform int num_diffuse_textures;
uniform int diffuse_index;

uniform sampler2D texture_normal;
uniform float normal_map_scale;
uniform int use_normal_map;

uniform sampler2D texture_metallic;
uniform float metallic_map_scale;
uniform int use_metallic;

struct Lighting {
	vec3 specular;
	vec3 diffuse;
	vec3 ambient;
};

// from file "shading.frag"
Lighting shading(vec3 position, float metallic, vec3 normal);
vec3 dither();

float triplanarMetallic() {
	float texture_scale = metallic_map_scale;
	float sharpness = 10.0;

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
		-tangent,
		-normalize(cross(tangent, vert_normal)), 
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

// https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
float mod289(float x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 mod289(vec4 x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 perm(vec4 x){return mod289(((x * 34.0) + 1.0) * x);}
float noise(vec3 p){
    vec3 a = floor(p);
    vec3 d = p - a;
    d = d * d * (3.0 - 2.0 * d);
    vec4 b = a.xxyy + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(b.xyxy);
    vec4 k2 = perm(k1.xyxy + b.zzww);
    vec4 c = k2 + a.zzzz;
    vec4 k3 = perm(c);
    vec4 k4 = perm(c + 1.0);
    vec4 o1 = fract(k3 * (1.0 / 41.0));
    vec4 o2 = fract(k4 * (1.0 / 41.0));
    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);
    return o4.y * d.y + o4.x * (1.0 - d.y);
}
vec3 fakeCubeMap(vec3 dir) {
	float d = 0.1;
    float l = 0.4;
    float n = (l - d) * pow(noise(3.*dir), 2.0) + d;
	return n.rrr;
}

void main() {

	float metallic = 0.;
	if(use_metallic != 0) 
	{
		metallic = triplanarMetallic();
	}
	vec3 normal = v_normal;
	if(use_normal_map != 0)
	{
		normal = triplanarNormal();
	}
	// calculate diffuse texture coords
	vec2 tex = v_tex;
	float mat_dist = 1.0/float(num_diffuse_textures);
	float mat_offset = mat_dist * float(diffuse_index);
	tex.x = tex.x * mat_dist + mat_offset;

	Lighting lighting = shading(frag_pos, metallic, normal);
	/*
	lighting.ambient = vec3(0.1);
	lighting.diffuse = vec3(0);
	lighting.specular = vec3(0);
	*/
	
	vec3 shading = vec3(0);
	shading += lighting.ambient;
	shading += lighting.diffuse;
	shading += lighting.specular;

	vec4 surface_color = texture(texture_diffuse, tex);
	float alpha = surface_color.a;

	float emission_strength = 0.0;
	if(use_emissive != 0)
	{
		emission_strength = texture(texture_emissive, v_tex).r;
	}

	vec3 iron_color = vec3(0.8862745098039216, 0.8862745098039216, 0.82352941176);
	surface_color.rgb = mix(surface_color.rgb, iron_color, metallic*(1-emission_strength));
	
	vec3 emission = emission_strength * surface_color.rgb;

	vec3 spec_emission = 1.*lighting.specular * surface_color.rgb;
	emission += spec_emission * (1.-emission_strength);

	vec3 view_dir = normalize(cam_position - frag_pos);

	vec3 color = surface_color.rgb;
	color *= mix(shading, vec3(1), emission_strength);
	if(use_metallic != 0)
	{
		color += fakeCubeMap(reflect(view_dir, normal)) * metallic*(1-emission_strength);
	}
	color += dither();
	//float gamma = 2.2;
    //color = pow(color, vec3(gamma));

	out_color = vec4(color, alpha);
	out_emission = vec4(emission, 1);
	
	//out_color = vec4(1,0,0,1);
	//out_emission = vec4(0,0,0,1);

	float depth = gl_FragCoord.z;
	out_depth = vec4(depth,0,0,0);
	out_normal = vec4(normalize(v_normal), 1);
	out_pos = vec4(frag_pos.xyz, 1);
}
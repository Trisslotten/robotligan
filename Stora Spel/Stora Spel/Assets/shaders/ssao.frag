#version 440 core

layout(location = 0) out vec4 out_occlusion;

in vec2 v_uv;
in vec2 v_pos;

uniform sampler2D texture_depth;
uniform sampler2D texture_normals;
uniform sampler2D texture_noise;
uniform sampler2D texture_position;

uniform vec3 samples[64];
uniform mat4 projection;
uniform mat4 inv_projection;

uniform vec2 screen_dims;

const float radius = 0.4;
const float bias = 0.00004;

void main() {
	vec2 noise_scale = vec2(screen_dims.x / 16.0, screen_dims.y / 16.0);

	float depth = texture(texture_depth, v_uv).r;
	vec3 frag_pos = texture(texture_position, v_uv).xyz;
	vec3 normal = texture(texture_normals, v_uv).rgb;
	normal = normalize(normal);

	vec3 random_vec = texture(texture_noise, v_uv * noise_scale).xyz;

	vec3 tangent   = normalize(random_vec - normal * dot(random_vec, normal));
	vec3 bitangent = cross(tangent,normal);
	mat3 TBN       = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for(int i = 0; i < 32; i++)
	{
		// get sample position
		vec3 samp = TBN * samples[i];
		samp = frag_pos + samp * radius;
		
		vec4 offset = vec4(samp, 1.0);
		offset = projection * offset;
		offset.xyz /= offset.w;
		offset.xyz  = offset.xyz * 0.5 + 0.5;

		float samp_depth = texture(texture_depth, offset.xy).r;
		float rangeCheck = smoothstep(0.0, 1.0, radius*0.01 / abs(depth - samp_depth));
		
		occlusion += (samp_depth + bias <= offset.z  ? 1.0 : 0.0) * rangeCheck;  
	}
	//occlusion*=2.0;
	occlusion = 1- (occlusion /32.0);
	//occlusion += 0.08;
	occlusion = pow(occlusion, 2);
	out_occlusion.r = min(1.0,occlusion);
}

// -- based on https://learnopengl.com/Advanced-Lighting/SSAO
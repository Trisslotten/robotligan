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

void main() {
	//vec3 color = texture(texture_color, v_uv).rgb;
	vec2 noise_scale = vec2(screen_dims.x / 4.0, screen_dims.y / 4.0);

	float depth = texture(texture_depth, v_uv).r;
	//vec4 fragPos4   = inv_projection * vec4(v_pos, depth, 1);
	//vec3 fragPos = fragPos4.xyz / fragPos4.w;

	vec3 fragPos = texture(texture_position, v_uv).xyz;

	//vec4 normal4    = projection * vec4(texture(texture_normals, v_uv).rgb,0);
	vec3 normal = texture(texture_normals, v_uv).rgb;//normalize(normal4.xyz);
	//normal.z *= -1.;
	normal = normalize(normal);
	vec3 randomVec = texture(texture_noise, v_uv * noise_scale).xyz;

	vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(tangent,normal);
	mat3 TBN       = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	float radius = 0.8;
	float bias = 0.00001;
	for(int i = 0; i < 8; i++)
	{
		// get sample position
		vec3 samp = TBN * samples[i]; // From tangent to view-space
		samp = fragPos + samp * radius;
		
		// ????
		vec4 offset = vec4(samp, 1.0);
		offset = projection * offset;    // from view to clip-space
		offset.xyz /= offset.w;               // perspective divide
		offset.xyz  = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0 
		// ????

		float samp_depth = texture(texture_depth, offset.xy).r;
		float rangeCheck = smoothstep(0.0, 1.0, radius*0.01 / abs(depth - samp_depth));
		
		
		//occlusion += (samp_depth >= offset.z + bias ? 1.0 : 0.0) * rangeCheck;  
		occlusion += (samp_depth + bias <= offset.z  ? 1.0 : 0.0) * rangeCheck;  
	}
	occlusion = 1- (occlusion /8.0);
	occlusion += 0.05;
	out_occlusion.r = min(1.0,occlusion);
}
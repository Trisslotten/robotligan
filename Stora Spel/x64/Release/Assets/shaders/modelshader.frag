#version 440 core

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_emission;
layout(location = 2) out vec4 out_normal;
layout(location = 3) out float out_depth;

in vec3 frag_pos;
in vec2 v_tex;
in vec3 v_normal;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;

// from file "shading.frag"
vec3 shading(vec3 position, vec3 normal);
vec3 dither();

void main()
{
	vec3 normal = normalize(v_normal);
	vec3 surface_color = texture(texture_diffuse, v_tex).rgb;

	vec3 color = surface_color * shading(frag_pos, normal);	

	color += dither();

	out_color = vec4(color, 1);
	out_emission = vec4(0,0,0,1);

	float depth = gl_FragCoord.z;
	out_depth = depth;
	out_normal = vec4(normal, 1);
}
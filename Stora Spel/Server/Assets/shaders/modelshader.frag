#version 440 core

out vec4 outColor;

in vec3 frag_pos;
in vec2 v_tex;
in vec3 v_normal;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;

uniform vec3 light_pos[64];
uniform vec3 light_col[64];
uniform float light_radius[64];
uniform float light_amb[64];

uniform float num_frames;

uniform int NR_OF_LIGHTS;

// https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
float rand(vec2 n) { 
	n = mod(n, vec2(20000));
	return 2*fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453)-1;
}

vec3 dither()
{
	vec3 result = vec3(0);
	float num_colors = 256.0;
	float val = 0.5*rand(gl_FragCoord.xy + num_frames*1000.0)/num_colors;
	result += val;
	return result;
}

void main()
{
	vec3 normal = normalize(v_normal);
	vec3 t_col = texture(texture_diffuse, v_tex).rgb;
	vec3 lighting = vec3(0);

	for(int l = 0; l < NR_OF_LIGHTS; l++){
		vec3 pointToLight = light_pos[l] - frag_pos;
		float intensity = 1.f - clamp(length(pointToLight), 0, light_radius[l]) / light_radius[l];
		vec3 light_dir = normalize(pointToLight);

		vec3 dir_color = max(dot(normalize(light_dir), normal), 0) * light_col[l] * intensity * t_col;
		vec3 amb_color = light_amb[l] * t_col;
		vec3 final_color = dir_color + amb_color;

		lighting += final_color;
	}


	lighting += dither();

	outColor = vec4(lighting, 1);
}
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

uniform int NR_OF_LIGHTS;

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

	outColor = vec4(lighting, 1);
}
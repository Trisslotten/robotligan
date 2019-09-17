#version 440 core

out vec4 outColor;

in vec3 frag_pos;
in vec2 v_tex;
in vec3 v_normal;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;

uniform vec3 light_pos;
uniform vec3 light_col;
uniform float light_radius;
uniform float light_amb;

void main()
{
	
	vec3 pointToLight = light_pos - frag_pos;
	float intensity = 1.f - clamp(length(pointToLight), 0, light_radius) / light_radius;
	vec3 light_dir = normalize(pointToLight);

	vec3 normal = normalize(v_normal);
	vec3 color = light_col * intensity * texture(texture_diffuse, v_tex).rgb;
	vec3 amb_color = light_amb * texture(texture_diffuse, v_tex).rgb;
	vec3 final_color = color + amb_color;


	vec3 lighting = vec3(0);
	lighting += max(dot(normalize(light_dir), normal), 0) * final_color;

	outColor = vec4(lighting, 1);
}
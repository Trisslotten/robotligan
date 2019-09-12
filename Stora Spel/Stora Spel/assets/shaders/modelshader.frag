#version 440 core

out vec4 outColor;

in vec2 v_tex;
in vec3 v_normal;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;

void main()
{
	vec3 normal = normalize(v_normal);
	vec3 color = texture(texture_diffuse, v_tex).rgb;

	vec3 light_dir = normalize(vec3(-1,1,0));

	vec3 lighting = vec3(0);
	lighting += max(dot(light_dir, normal), 0) * color;
	lighting += 0.1 * color;

	outColor = vec4(lighting, 1);
}
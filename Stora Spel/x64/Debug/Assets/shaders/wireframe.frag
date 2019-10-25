#version 440 core

out vec4 outColor;

in vec2 v_tex;
in vec3 v_normal;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;

void main()
{
	outColor = vec4(1, 0, 0, 1);
}
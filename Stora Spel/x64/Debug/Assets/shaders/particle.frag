#version 440 core

layout (location = 0) in vec4 color;
layout (location = 1) in vec2 tex_coord;

out vec4 outColor;

//uniform vec4 color;
uniform sampler2D image;

void main()
{
	vec4 tex_color = texture(image, tex_coord);
	if (tex_color.a == 0.f)
		discard;

	outColor = vec4(color * tex_color);
}
#version 440 core

layout (location = 0) in vec4 color;
layout (location = 1) in vec2 tex_coord;

out vec4 outColor;

//uniform vec4 color;
uniform sampler2D ourTexture;

void main()
{
	vec4 tex_color = texture(ourTexture, tex_coord);
	if (tex_color.a == 0.f)
		discard;
	//tex_color.a = 1.0 - tex_color.a;
	//tex_color.a = clamp(tex_color.a, 0.3f, 0.8);
	tex_color.a = tex_color.a * 0.8f;
	outColor = vec4(color * tex_color);
}
#version 440 core

layout (location = 0) in vec4 color;
layout (location = 1) in vec2 tex_coord;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_emission;

//uniform vec4 color;
uniform sampler2D image;
uniform bool emissive;

void main()
{
	vec4 tex_color = texture(image, tex_coord);
	if (tex_color.a == 0.f)
		discard;

	vec4 c = vec4(color * tex_color);
	out_color = c;
	//c.a = 1.0;
	if (!emissive)
		c = vec4(0);

	//c = c * c.a;
	out_emission = c;
}
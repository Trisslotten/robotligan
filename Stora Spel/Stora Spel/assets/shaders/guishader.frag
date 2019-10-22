#version 440 core

in vec2 tex_coords;
out vec4 frag_color;

uniform sampler2D gui_element_texture;
uniform float opacity;

void main()
{
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(gui_element_texture, tex_coords).a);
	frag_color = texture(gui_element_texture, tex_coords) * sampled;
	frag_color.a *= opacity;
}
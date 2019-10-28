#version 440 core

in vec2 tex_coords;
out vec4 frag_color;

uniform sampler2D gui_element_texture;
uniform float opacity;

void main()
{
	frag_color = texture(gui_element_texture, tex_coords);
	frag_color.a *= opacity;
}
#version 440 core

out vec4 outColor;

in vec2 v_uv;

void main()
{
	vec2 pixel = (gl_FragCoord.xy + 0.5)/100.0;
	float color = mod(ceil(pixel.x) + ceil(pixel.y), 2.f);
	outColor = vec4(vec3(color), 1);
}
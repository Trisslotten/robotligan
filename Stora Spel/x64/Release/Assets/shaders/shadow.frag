#version 440 core

out vec4 depths;

in vec3 frag_pos;

uniform vec3 shadow_light_pos;

const float near = 0.1;
const float far = 300.0;
float linearDepth(float depth)
{
	return far*near/(depth * (near - far) + far);
}

void main() {
	//float depth = length(frag_pos - shadow_light_pos);
	float depth = linearDepth(gl_FragCoord.z);
	depths = vec4(depth, depth*depth, 0, 0);
}

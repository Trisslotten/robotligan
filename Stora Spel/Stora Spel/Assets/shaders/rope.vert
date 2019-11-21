#version 440 core

layout(location = 0) in vec2 pos;

uniform mat4 cam_transform;

out vec2 v_uv;

const float PI = 3.14159265359;

void main() {
	v_uv = pos;

	float a = 2.*PI*pos.y;
	vec3 cylinder = vec3(0.5,0.1,0.1) *vec3(pos.x, 0.5*cos(a), 0.5*sin(a));

	gl_Position = cam_transform * vec4(cylinder + vec3(3),1);
}
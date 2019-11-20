#version 440 core

layout(location = 0) in vec4 pos;

uniform mat4 cam_transform;

out vec2 v_uv;

void main() {
	v_uv = pos.xw;
	gl_Position = cam_transform * vec4(pos.xyz + vec3(3),1);
}
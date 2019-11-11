#version 440 core

layout(location = 0) in vec3 pos;

out vec2 uv;
out vec4 proj_pos;

uniform mat4 view;
uniform mat4 projection;


void main() {
	uv = (pos.xy+1.0)/2.0;

	vec2 scale = vec2(1.5, 1);
	proj_pos = projection * view * (vec4(pos.x, 1., pos.y, 1.) * scale.xyxy);
	gl_Position = proj_pos.xyww;
}
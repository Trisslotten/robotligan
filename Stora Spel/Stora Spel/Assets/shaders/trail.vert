#version 440 core

#define MAX_POSITIONS 100

layout(location = 0) in vec3 pos;

uniform mat4 cam_transform;
uniform vec3 cam_pos;

uniform vec3 position_history[MAX_POSITIONS];
uniform float width;


void main() {
	int index = int(floor(pos.x * (MAX_POSITIONS-1)));
	float t = fract(pos.x * (MAX_POSITIONS-1));

	vec3 p1 = position_history[index];
	vec3 p2 = position_history[index+1];

	vec3 wpos = mix(p1, p2, t);

	vec3 dir = p1 - p2;
	vec3 cdir = wpos - cam_pos;
	vec3 tangent = normalize(cross(dir, cdir));

	float side = width * pos.z * (1.0-pos.x);

	wpos = wpos + side * tangent;

	gl_Position = cam_transform * vec4(wpos, 1);
}
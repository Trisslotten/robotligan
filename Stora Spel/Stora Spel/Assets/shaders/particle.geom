#version 440 core

layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

layout (location = 0) in vec3 g_pos[];

uniform mat4 cam_transform;

void main()
{
	vec3 right = vec3(1.0, 0.0, 0.0);
	vec3 up = vec3(0.f, 1.f, 0.f);

	vec3 world_pos = g_pos[0] + right + up;
	gl_Position = cam_transform * vec4(world_pos, 1.0);
	EmitVertex();
	world_pos = g_pos[0] + right - up;
	gl_Position = cam_transform * vec4(world_pos, 1.0);
	EmitVertex();
	world_pos = g_pos[0] - right - up;
	gl_Position = cam_transform * vec4(world_pos, 1.0);
	EmitVertex();

	EndPrimitive();

	world_pos = g_pos[0] - right + up;
	gl_Position = cam_transform * vec4(world_pos, 1.0);
	EmitVertex();
	world_pos = g_pos[0] + right + up;
	gl_Position = cam_transform * vec4(world_pos, 1.0);
	EmitVertex();
	world_pos = g_pos[0] - right - up;
	gl_Position = cam_transform * vec4(world_pos, 1.0);
	EmitVertex();

	EndPrimitive();
}
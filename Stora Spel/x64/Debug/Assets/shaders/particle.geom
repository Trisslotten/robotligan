#version 440 core

layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

layout (location = 0) in vec3 g_pos[];
layout (location = 1) in vec4 g_color[];
layout (location = 2) in float g_size[];
layout (location = 3) in float g_time[];

layout (location = 0) out vec4 color;
layout (location = 1) out vec2 tex_coord;

uniform mat4 cam_transform;
uniform vec3 cam_pos;
uniform vec3 cam_up;
uniform float size;

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

void main()
{
	if (g_time[0] > 0.0) {

		color = g_color[0];
		vec3 point_to_cam = cam_pos - g_pos[0];
		//mat4 rotate = rotationMatrix(point_to_cam, g_time[0]);
		
		vec3 right = cross(cam_up, point_to_cam);
		vec3 up = cross(right, point_to_cam);

		up = normalize(up) * g_size[0];
		right = normalize(right) * g_size[0];

		//up = (rotate * vec4(up, 0)).xyz;
		//right = (rotate * vec4(right, 0)).xyz;
		if (g_pos[0].y == -11.0) {
			up = vec3(1,0.2,0) * g_size[0];
			right = vec3(0,0.2,1) * g_size[0];
		}

		vec3 world_pos = g_pos[0] + right + up;
		gl_Position = cam_transform * vec4(world_pos, 1.0);
		tex_coord = vec2(1.0, 1.0);
		EmitVertex();
		world_pos = g_pos[0] + right - up;
		gl_Position = cam_transform * vec4(world_pos, 1.0);
		tex_coord = vec2(1.0, 0.0);
		EmitVertex();

		world_pos = g_pos[0] - right + up;
		gl_Position = cam_transform * vec4(world_pos, 1.0);
		tex_coord = vec2(0.0, 1.0);
		EmitVertex();
		
		world_pos = g_pos[0] - right - up;
		gl_Position = cam_transform * vec4(world_pos, 1.0);
		tex_coord = vec2(0.0, 0.0);
		EmitVertex();

		EndPrimitive();
	}
}
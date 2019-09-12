#version 440 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 tex;
layout(location = 2) in vec3 normal;

out vec2 v_tex;
out vec3 v_normal;

uniform mat4 cam_transform;
uniform mat4 model_transform;

void main()
{
	v_tex = tex;
	v_normal = normalize(transpose(inverse(mat3(model_transform)))*normal);
	gl_Position = cam_transform * model_transform * vec4(pos, 1.0);
}
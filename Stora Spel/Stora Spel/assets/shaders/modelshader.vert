#version 440 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 tex;
layout(location = 2) in vec3 normal;

out vec3 local_pos;
out vec3 local_normal;
out vec3 frag_pos;
out vec2 v_tex;
out vec3 v_normal;


uniform mat4 cam_transform;
uniform mat4 model_transform;
uniform mat4 mesh_transform;
uniform mat3 normal_transform;

// shading.vert
void handleShading(vec3 position);

void main() {
	frag_pos = (model_transform * (mesh_transform * vec4(pos, 1.0))).xyz;
	v_tex = tex;
	v_normal = normalize(normal_transform*normal);
	
	local_pos = pos;
	local_normal = normal;

	handleShading(frag_pos);

	gl_Position = cam_transform * vec4(frag_pos, 1.0);
}
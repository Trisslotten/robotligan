#version 440 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 tex;
layout(location = 2) in vec3 normal;
layout(location = 3) in ivec4 bones;
layout(location = 4) in vec4 weights;

out vec3 local_pos;
out vec3 local_normal;
out vec3 frag_pos;
out vec2 v_tex;
out vec3 v_normal;

uniform mat4 cam_transform;
uniform mat4 model_transform;
uniform mat4 bone_transform[64];

//uniform int NR_OF_BONES;

// shading.vert
void handleShading(vec3 position);

void main()
{
    vec4 t_vertex = vec4(pos, 1.f);
	vec4 t_normal = vec4(0.f);
	mat4 anim_transform = mat4(0.f);

	int i = 0;
	while(bones[i] != -1 && i < 4){
		anim_transform += (bone_transform[bones[i]] * weights[i]);
		i++;
	}
	mat4 transform = model_transform * anim_transform;

	local_pos = pos;
	local_normal = normal;

	t_vertex = transform * t_vertex;
	v_normal = normalize(transpose(inverse(mat3(transform))) * normal.xyz);
	v_tex = tex;
	frag_pos = t_vertex.xyz;

	handleShading(t_vertex.xyz);

	gl_Position = cam_transform * t_vertex;
}
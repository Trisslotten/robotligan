#version 440 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 tex;
layout(location = 2) in vec3 normal;
layout(location = 3) in ivec4 bones;
layout(location = 4) in vec4 weights;

out vec3 frag_pos;
out vec2 v_tex;
out vec3 v_normal;

uniform mat4 cam_transform;
uniform mat4 model_transform;
uniform mat4 bone_transform[64];

//uniform int NR_OF_BONES;

void main()
{
    vec4 t_vertex = vec4(0.f);
	vec4 t_normal = vec4(0.f);

	int i = 0;

	t_vertex = (weights[0] * bone_transform[bones[0]]) * vec4(pos, 1.f);

	//while(bones[i] != -1 && i < 4){
		//t_vertex = (weights[i] * bone_transform[bones[i]]) * vec4(pos, 1.f);
		//i++;
	//}

	frag_pos = (model_transform * t_vertex).xyz;

	v_tex = tex;
	v_normal = normalize(transpose(inverse(mat3(model_transform)))*normal);
	gl_Position = cam_transform * model_transform * t_vertex;
}
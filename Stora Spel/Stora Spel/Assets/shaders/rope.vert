#version 440 core

layout(location = 0) in vec2 pos;

uniform mat4 cam_transform;

uniform vec3 start_pos;
uniform vec3 end_pos;

out vec2 v_uv;
out vec3 v_normal;

const float TWO_PI = 2.*3.14159265359;

// http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
mat3 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    return mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c);
}

void main() {
	vec3 vec = end_pos - start_pos;
	vec3 axis = cross(normalize(vec), vec3(1,0,0));
	float angle = acos(normalize(vec).x);
	mat3 rot = mat3(1.0);
	if(length(axis) > 0.001) {
		rot = rotationMatrix(axis, angle);
	}
	v_uv = vec2(length(vec) * (1-pos.x), pos.y);

	float a = TWO_PI * pos.y;
	vec3 cylinder = vec3(1.0, 0.1, 0.1) *vec3(pos.x, 0.5*cos(a), 0.5*sin(a));
	vec3 normal = normalize(vec3(0, cylinder.yz));

	v_normal = normal;

	cylinder *= vec3(length(vec), 1, 1);
	cylinder = rot * cylinder;
	cylinder += start_pos;

	gl_Position = cam_transform * vec4(cylinder,1);
}
#version 440 core

uniform vec3 light_pos[64];
uniform vec3 light_col[64];
uniform float light_radius[64];
uniform float light_amb[64];
uniform int NR_OF_LIGHTS;


// https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
float rand(vec2 n) { 
	n = mod(n, vec2(20000));
	return 2*fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453)-1;
}

vec3 dither() {
	vec3 result = vec3(0);
	float num_colors = 256.0;
	float val = 0.5*rand(gl_FragCoord.xy)/num_colors;
	result += val;
	return result;
}

vec3 shading(vec3 position, vec3 normal) {
	vec3 lighting = vec3(0);
	for(int l = 0; l < NR_OF_LIGHTS; l++){
		vec3 pointToLight = light_pos[l] - position;
		vec3 light_dir = normalize(pointToLight);

		float intensity = 1.f - clamp(length(pointToLight), 0, light_radius[l]) / light_radius[l];

		vec3 diffuse = max(dot(light_dir, normal), 0) * light_col[l] * intensity;
		float ambient = light_amb[l];

		lighting += (diffuse + ambient);
	}
	return lighting;
}
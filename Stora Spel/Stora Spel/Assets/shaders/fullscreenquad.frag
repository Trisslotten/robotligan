#version 440 core

out vec4 out_color;

in vec2 v_uv;

uniform sampler2D texture_color;
uniform sampler2D texture_emission;
uniform sampler2D texture_ssao;

uniform bool use_ao;

uniform int is_invisible;

// https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}
float noise(vec2 p){
	vec2 ip = floor(p);
	vec2 u = fract(p);
	u = u*u*(3.0-2.0*u);
	
	float res = mix(
		mix(rand(ip),rand(ip+vec2(1.0,0.0)),u.x),
		mix(rand(ip+vec2(0.0,1.0)),rand(ip+vec2(1.0,1.0)),u.x),
		u.y);
	return res*res;
}

void main() {
	vec3 color = texture(texture_color, v_uv).rgb;

	vec4 emission = texture(texture_emission, v_uv, 1);
	float ao = texture(texture_ssao, v_uv).r;
	if(use_ao) color*=ao;
	color += emission.rgb*0.9;
	
	if (is_invisible == 1) {
		vec3 effect = vec3(noise(gl_FragCoord.xy/100.0));
		effect *= vec3(0.85,0.85,1);
		color = mix(color, effect, smoothstep(0.35,0.85, length(v_uv-0.5)));
	}

	out_color = vec4(color, 1);
	//if(use_ao) out_color = vec4(vec3(ao),1);
}
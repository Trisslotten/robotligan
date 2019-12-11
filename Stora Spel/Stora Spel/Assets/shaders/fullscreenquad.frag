#version 440 core

out vec4 out_color;

in vec2 v_uv;

uniform sampler2D texture_color;
uniform sampler2D texture_emission;
uniform sampler2D texture_ssao;

uniform bool use_ao;
uniform int is_invisible;
uniform int stunned;
uniform float time;

uniform vec2 resolution;

#define MAX_SHOCKWAVES 8
uniform int shockwave_count;
uniform vec3 shockwave_positions[MAX_SHOCKWAVES];
uniform float shockwave_time_ratios[MAX_SHOCKWAVES];
uniform float shockwave_radii[MAX_SHOCKWAVES];


#define MAX_BLACK_HOLES 8
uniform int blackhole_count;
uniform vec3 blackhole_positions[MAX_BLACK_HOLES];
uniform float blackhole_strengths[MAX_BLACK_HOLES];
uniform float blackhole_radii[MAX_BLACK_HOLES];

// https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}
float rand(float n) { 
	return fract(sin(n * 15.9898) * 53758.5453);
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

vec3 dither() {
	const float num_colors = 256.0;
	return vec3(0.5*rand(gl_FragCoord.xy)/num_colors);
}

float noise(float p){
	float ip = floor(p);
	float  u = fract(p);
	u = u*u*(3.0-2.0*u);
	float res = mix(rand(ip),rand(ip+1.0),u);
	return res*res;
}

vec3 calcColor(vec2 uv) {
	vec3 color = texture(texture_color, uv).rgb;

	vec4 emission = texture(texture_emission, uv, 1);
	float ao = texture(texture_ssao, uv).r;
	if(use_ao) color*=ao;
	color += emission.rgb;
	
	if (is_invisible == 1) {
		vec3 effect = vec3(noise(gl_FragCoord.xy/100.0));
		effect *= vec3(0.85,0.85,1);
		color = mix(color, effect, smoothstep(0.35,0.85, length(uv-0.5)));
	}
	return color;
}

void main() {
	vec2 uv = v_uv;
	
	for(int i = 0; i < blackhole_count; i++) {
		vec2 pos = blackhole_positions[i].xy * resolution;
		float radius = blackhole_radii[i];
		vec2 vec = gl_FragCoord.xy-pos;
		float dist = length(vec);
		vec2 disp_dir = normalize(gl_FragCoord.xy-pos);
		float strength = clamp(dist / radius, 0., 1.);
		strength = pow(strength-1, 2);
		uv += 0.3*disp_dir * strength * blackhole_strengths[i];
	}
	for(int i = 0; i < shockwave_count; i++) {
		vec2 pos = shockwave_positions[i].xy * resolution;
		float thickness_ratio = 0.5;
		float radius = shockwave_radii[i];
		float shockwave = smoothstep(radius, radius-2.0, length(pos - gl_FragCoord.xy));
		shockwave *= smoothstep((radius-2.)*thickness_ratio, radius-2.0,length(pos - gl_FragCoord.xy));
		shockwave *= 1.-shockwave_time_ratios[i];
		vec2 disp_dir = normalize(gl_FragCoord.xy-pos);
		uv += 0.02*disp_dir*shockwave;
	}

	vec2 n = vec2(0);
	vec3 noise_color;
	float t = 0;

	if(stunned != 0) 
	{
		n.x = 0.02*noise(40*time + 0.02);
		noise_color.r = rand(gl_FragCoord.xy + time);
		noise_color.g = rand(gl_FragCoord.xy + 1000 + time);
		noise_color.b = rand(gl_FragCoord.xy + 2000 + time);
		t = 0.20;
	}
	vec3 color;
	color.r = calcColor(uv + n).r;
	color.g = calcColor(uv).g;
	color.b = calcColor(uv - n).b;
	color = mix(color, noise_color, t);

	color = pow(color, vec3(1/2.2));

	color += dither();

	//color = texture(texture_ssao, v_uv).rrr;
	out_color = vec4(color, 1);
	//if(use_ao) out_color = vec4(vec3(ao),1);
}
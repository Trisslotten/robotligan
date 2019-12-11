#version 440 core

uniform sampler2D msdf;
uniform float pxRange;
uniform vec4 fgColor;


in vec2 v_tex;

out vec4 color;
layout(location = 1) out vec4 out_emission;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
	vec2 msdfUnit = 10./vec2(textureSize(msdf, 0));
	//vec2 msdfUnit = pxRange/vec2(200, 200);
	vec2 vv_tex = v_tex;
	vv_tex.y = 1 - v_tex.y;
	vec3 samplep = texture(msdf, vv_tex).rgb;
    float sigDist = median(samplep.r, samplep.g, samplep.b) - 0.5;
	sigDist *= dot(msdfUnit, 0.5/fwidth(v_tex));
    float opacity = clamp(sigDist + 0.5, 0.0, 1.0);

	if (opacity < .05)
		discard;

    color = vec4(fgColor.rgb, opacity);
	out_emission = vec4(0);
}
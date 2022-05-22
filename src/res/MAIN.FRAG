#version 100
precision lowp float;
uniform vec4 _ColorAdd, _Color, _ColorFog;
uniform sampler2D _MainTexture, _FramebufferTexture;
uniform float _IOR, _Refractive, _AlphaBlend;

varying vec2 _UVs;
varying vec3 _ViewDir, _NormalDir;

// Turn -1.0/1.0 to 0.0/1.0
#define unsign(x) (((x) + 1.0) / 2.0)

void main(){
	vec2 refD = refract(_ViewDir, _NormalDir, _IOR).xy;
	refD = vec2(unsign(refD.x), unsign(refD.y));

	vec2 sUVs = vec2(_UVs.x, 1.0 - _UVs.y);
	float t_c = texture2D(_MainTexture, sUVs).r;
	vec4 main_color = texture2D(_MainTexture, sUVs) * _Color;
	vec4 fb_color = texture2D(_FramebufferTexture, refD);
	vec4 final_color = main_color;
	final_color.rgb = mix(main_color, fb_color, main_color.a).rgb;
	final_color = mix(main_color, final_color, _Refractive);
	final_color.rgb += _ColorAdd.rgb * _ColorAdd.a;
	final_color.rgb = mix(final_color.rgb, _ColorFog.rgb, pow(_ViewDir.z * 1.2, 8.0) * _ColorFog.a);
	final_color.a = mix(1.0, main_color.r, min(max(_AlphaBlend, 0.0), 1.0));
	final_color.a = mix(final_color.a, t_c, min(max(_AlphaBlend - 1.0, 0.0), 1.0));
	gl_FragColor = final_color;
}
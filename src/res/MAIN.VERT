#version 100
precision highp float;

attribute vec3 POSITION, NORMAL;
attribute vec2 TEXCOORD;

uniform mat4 _Transform;

varying vec2 _UVs;
varying vec3 _ViewDir, _NormalDir;

void main(){
	vec4 pos = _Transform * vec4(POSITION, 1.0);
	gl_Position = pos;
	_ViewDir = normalize(pos.xyz);
	_UVs = TEXCOORD;
	// _NormalDir = (_Transform * vec4(normalize(NORMAL * 1.0), 1.0)).xyz;
	_NormalDir = normalize(NORMAL);
}
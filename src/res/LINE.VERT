#version 100
precision highp float;
// Total Size 8 floats
attribute vec3 POSITION;
attribute float ALPHA;

uniform mat4 _Transform;

varying float _Alpha;

void main(){
	gl_Position =  _Transform * vec4(POSITION, 1.0);
	_Alpha = ALPHA;
}
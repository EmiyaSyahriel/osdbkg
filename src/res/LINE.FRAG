#version 100
precision lowp float;
uniform vec3 _Color;

varying float _Alpha;

void main(){
	gl_FragColor = vec4(_Color, _Alpha);
}
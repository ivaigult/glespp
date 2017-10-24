#version 100

attribute vec3 aPos;
attribute vec3 aNorm;

uniform mat4 uMVP;
uniform mat4 uMV;
uniform mat4 uNormal;

varying vec3 vPos;
varying vec3 vNorm;

void main()
{
	vNorm = (uNormal * vec4(aNorm, 0.0)).xyz;
	vPos  = (uMV * vec4(aPos, 1.0)).xyz;
	gl_Position = uMVP * vec4(aPos, 1.0);
}

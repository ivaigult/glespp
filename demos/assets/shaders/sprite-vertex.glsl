#version 110
uniform mat4      MVP;

attribute vec2 vPos;
attribute vec2 vTex;

varying vec2 varTexCoord;

void main()
{
    gl_Position = MVP * vec4(vPos, 0.0, 1.0);
	varTexCoord = vTex;
}

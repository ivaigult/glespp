#version 130 
uniform mat4 MVP;

attribute vec3 vPos;
attribute vec3 vCol;
varying   vec3 varCol;

void main()
{
    gl_Position = MVP * vec4(vPos, 1.0);
	varCol = vCol;
}

#version 130

varying   vec3 varCol;

void main()
{
    gl_FragColor = vec4(varCol, 1.0);
}

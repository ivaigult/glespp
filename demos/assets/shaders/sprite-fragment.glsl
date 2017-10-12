#version 110

varying vec2 varTexCoord;
uniform sampler2D uTex;

void main()
{
    gl_FragColor = texture2D(uTex, varTexCoord);
}

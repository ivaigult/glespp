#version 100

struct light {
	vec4 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

uniform light light0;

varying   vec3 varCol;

void main()
{
    gl_FragColor = vec4(varCol + light0.specular.r, 1.0);
}

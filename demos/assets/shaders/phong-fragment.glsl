#version 100

struct light {
    vec4 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

struct material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

uniform material  uMaterial;
uniform light     uLight;

varying vec3 vPos;
varying vec3 vNorm;

void main()
{
	// ambient-term
	vec4 ambient_term = uMaterial.ambient;

	// diffuse-term
	vec3 lightDir = normalize(uLight.position.xyz - vPos);
	vec4 diff_term = uMaterial.diffuse * max(dot(lightDir, vNorm), 0.0);
	diff_term = clamp(diff_term, 0.0, 1.0);

	// specular-term
	vec3 light_reflection = -reflect(lightDir, vNorm);
	vec3 eyeDir = normalize(-vPos);
	vec4 specular_term = uMaterial.specular * pow(max(dot(light_reflection, eyeDir), 0.0), 24.0);
	
    gl_FragColor = ambient_term + diff_term + specular_term;
}

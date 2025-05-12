#version 430

struct MaterialInfo{
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
};

uniform MaterialInfo Material;

in vec4 FragPosLightSpace;

uniform vec3 lightPos;
uniform float farPlane;

void main()
{
    float distance = length(FragPosLightSpace.xyz - lightPos);
    gl_FragDepth = distance / farPlane;
}
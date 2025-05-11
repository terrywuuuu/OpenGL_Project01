#version 430 core
out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;

uniform vec3 cameraPos;
uniform samplerCube environmentMap;

void main()
{
    vec3 I = normalize(WorldPos - cameraPos);  
    vec3 R = reflect(I, normalize(Normal));  
    vec3 reflectedColor = texture(environmentMap, R).rgb;

    FragColor = vec4(reflectedColor, 1.0);
}
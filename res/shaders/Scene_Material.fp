// ADS Point lighting Shader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible

#version 430
struct MaterialInfo{
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
};

uniform MaterialInfo Material;
out vec4 vFragColor;

uniform sampler2D shadowMap;
uniform float far_plane;
uniform mat4 lightSpaceMatrix;
uniform vec3 cameraPos;

//lighting color
vec4    ambientColor = vec4(0.1,0.1,0.1,1);
vec4    diffuseColor = vec4(0.8,0.8,0.8,1);   
vec4    specularColor = vec4(1,1,1,1);

in vec3 vVaryingNormal;
in vec3 vVaryingLightDir;
in vec2 UV;
in vec3 FragPos;
in vec3 lightPos;
float Shininess = 128.0;//for material specular

uniform sampler2D tex0;

float ShadowCalculation(vec3 fragPos)
{
    vec3 fragToLight = lightPos - fragPos;

    vec3 dir = normalize(fragToLight); 

    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // [-1, 1] -> [0, 1]

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = max(0.005 * (1.0 - dot(vVaryingNormal, dir)), 0.001);

    if (projCoords.z > 1.0) {
        return 0.0;
    }

    return (currentDepth > closestDepth) ? 0.5 : 0.0;
}

void main(void)
{
    vec4 textureColor = texture(tex0, UV);
    
    vec3 lightDir = normalize(lightPos - FragPos); 

    float shadow = ShadowCalculation(FragPos);

    float diff = max(0.0, dot(normalize(vVaryingNormal), lightDir));
    vec4 diffuse = diff * textureColor * vec4(Material.Kd, 1.0);

    vec4 ambient = ambientColor * vec4(Material.Ka, 1.0);

    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normalize(vVaryingNormal));
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), Shininess);
    vec4 specular = specularColor * vec4(Material.Ks, 1.0) * spec;

    // Combine lighting with shadow factor 
    vFragColor = ambient + ((1.0 - shadow * 0.8) * (diffuse + specular));
}
   
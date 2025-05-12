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

uniform samplerCube depthCubemap;
uniform float far_plane;

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
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);

    // normalize direction for cubemap lookup
    vec3 dir = normalize(fragToLight);
    float closestDepth = texture(depthCubemap, dir).r * far_plane;

    // bias with surface angle to reduce acne
    float bias = max(0.15 * (1.0 - dot(normalize(vVaryingNormal), dir)), 0.05);
    float shadow = currentDepth - bias > closestDepth ? 0.5 : 0.0;

    return shadow;
}

void main(void)
{ 
    vec4 textureColor = texture(tex0, UV);
    float shadow = ShadowCalculation(FragPos);
    vec3 lightDir = normalize(lightPos - FragPos);

	// Diffuse lighting
    float diff = max(0.0, dot(normalize(vVaryingNormal), lightDir));
    vec4 diffuse = diff * textureColor * vec4(Material.Kd, 1.0);

    // Ambient
    vec4 ambient = ambientColor * vec4(Material.Ka, 1.0);

    // Specular
    vec3 viewDir = normalize(-FragPos); // 簡單假設 camera 在原點
    vec3 reflectDir = reflect(-lightDir, normalize(vVaryingNormal));
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), Shininess);
    vec4 specular = specularColor * vec4(Material.Ks, 1.0) * spec;

    // Combine with shadow factor
    vFragColor = ambient + ((1.0 - shadow * 0.8) * (diffuse + specular));
}
	
    
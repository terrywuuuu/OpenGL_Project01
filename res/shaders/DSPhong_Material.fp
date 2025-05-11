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

//lighting color
vec4    ambientColor = vec4(0.1,0.1,0.1,1);
vec4    diffuseColor = vec4(0.8,0.8,0.8,1);   
vec4    specularColor = vec4(1,1,1,1);

uniform samplerCube depthCubemap;
uniform float far_plane;

in vec3 vVaryingNormal;
in vec3 vVaryingLightDir;
in vec2 UV;
in vec3 lightPos;
in vec3 FragPos;
float Shininess = 128.0;//for material specular

uniform bool isLightCube;

/*float ShadowCalculation(vec3 fragPos)
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
}*/

void main(void)
{ 
    if(isLightCube){
        vFragColor = vec4(1.0); // 白色    
        return;
    }

//    float shadow = ShadowCalculation(FragPos);

    // Dot product gives us diffuse intensity
    float diff = max(0.0, dot(normalize(vVaryingNormal),
					normalize(vVaryingLightDir)));

    // Multiply intensity by diffuse color, force alpha to 1.0
    vFragColor = diff * diffuseColor*vec4(Material.Kd,1);

    // Add in ambient light
    vFragColor += ambientColor;


    // Specular Light
    vec3 vReflection = normalize(reflect(-normalize(vVaryingLightDir),
								normalize(vVaryingNormal)));//反射角
    float spec = max(0.0, dot(normalize(vVaryingNormal), vReflection));
    if(diff != 0) {
		spec = pow(spec, Shininess);
		vFragColor += specularColor*vec4(Material.Ks,1)*spec;
    }

/*    // Diffuse lighting
    float diff = max(0.0, dot(normalize(vVaryingNormal), normalize(vVaryingLightDir)));
    vec4 diffuse = diff * diffuseColor * vec4(Material.Kd, 1.0);

    // Ambient
    vec4 ambient = ambientColor * vec4(Material.Ka, 1.0);

    // Specular
    vec3 viewDir = normalize(-FragPos); // 簡單假設 camera 在原點
    vec3 reflectDir = reflect(-normalize(vVaryingLightDir), normalize(vVaryingNormal));
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), Shininess);
    vec4 specular = specularColor * vec4(Material.Ks, 1.0) * spec;

    // Combine with shadow factor
    vFragColor = ambient + (1.0 - shadow) * (diffuse + specular);*/
}
	
    
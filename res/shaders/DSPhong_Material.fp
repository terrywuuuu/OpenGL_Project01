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
layout (location = 0) out vec4 vFragColor;
layout (location = 1) out vec2 MotionVector;

uniform MaterialInfo Material;

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

// for motion blur
in vec4 ClipSpacePos0;
in vec4 PrevClipSpacePos0;

float Shininess = 128.0;//for material specular

uniform bool isLightCube;
uniform bool enableToonShader;

void main(void)
{ 
    if(isLightCube){
        vFragColor = vec4(1.0); // 白色    
        MotionVector = vec2(0, 0);
        return;
    }

    // Dot product gives us diffuse intensity
    float diff = max(0.0, dot(normalize(vVaryingNormal),
					normalize(vVaryingLightDir)));
    float toonDiff = diff;

    if(enableToonShader){
        if (diff > 0.95)
            toonDiff = 1.0;
        else if (diff > 0.5)
            toonDiff = 0.7;
        else if (diff > 0.25)
            toonDiff = 0.4;
        else
            toonDiff = 0.1;   
    }

    // Multiply intensity by diffuse color, force alpha to 1.0
    vFragColor = toonDiff * diffuseColor*vec4(Material.Kd,1);

    // Add in ambient light
    vFragColor += ambientColor;


    // Specular Light
    vec3 vReflection = normalize(reflect(-normalize(vVaryingLightDir),
								normalize(vVaryingNormal)));//反射角
    float spec = max(0.0, dot(normalize(vVaryingNormal), vReflection));

    float toonSpec = spec;
    if (spec > 0.95 && enableToonShader)
        toonSpec = 1.0;

    if(diff != 0) {
		toonSpec = pow(toonSpec, Shininess);
		vFragColor += specularColor*vec4(Material.Ks,1)*toonSpec;
    }
    
    vec3 NDCPos = (ClipSpacePos0 / ClipSpacePos0.w).xyz;
    vec3 PrevNDCPos = (PrevClipSpacePos0 / PrevClipSpacePos0.w).xyz;
    MotionVector = (NDCPos - PrevNDCPos).xy;

}
	
    
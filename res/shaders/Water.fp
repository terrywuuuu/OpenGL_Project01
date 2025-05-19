#version 430 core

in vec4 clipSpace;
in vec2 textureCoords;
in vec3 toCamerVector;
in vec3 fromLightVector;

out vec4 out_Color;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D dudvMap;
uniform sampler2D normalMap;

uniform bool enableWave;
uniform bool enableLightReflection;

uniform float moveFactor;

const float waveStrength = 0.04;
const float shineDamper = 20.0;
const float reflectivity = 0.5;

void main(void) {
	//calc reflection, refraction texture coordinate
	vec2 ndc = (clipSpace.xy / clipSpace.w) / 2 + 0.5;
	vec2 reflectionCoords = vec2(ndc.x, -ndc.y);
	vec2 refractionCoords = vec2(ndc.x, ndc.y);

	//deal distorted
	vec2 distortedTexCoords = texture(dudvMap, vec2(textureCoords.x + moveFactor, textureCoords.y)).rg*0.1;
	distortedTexCoords = textureCoords + vec2(distortedTexCoords.x, distortedTexCoords.y+moveFactor);
	vec2 totalDistortion = (texture(dudvMap, distortedTexCoords).rg * 2.0 - 1.0) * waveStrength;

	if(enableWave) {
		reflectionCoords += totalDistortion;
		reflectionCoords.x = clamp(reflectionCoords.x, 0.001, 0.999);
		reflectionCoords.y = clamp(reflectionCoords.y, -0.999, -0.001);
		refractionCoords += totalDistortion;
		refractionCoords = clamp(refractionCoords, 0.001, 0.999);
	}
	
	vec4 reflectionColor = texture(reflectionTexture, reflectionCoords);
	vec4 refractionColor = texture(refractionTexture, refractionCoords);
	
	vec3 viewVector = normalize(toCamerVector);
	float refractiveFactor = dot(viewVector, vec3(0.0, 1.0, 0.0));
	refractiveFactor = pow(refractiveFactor, 2.0);
	
	out_Color = mix(reflectionColor, refractionColor, refractiveFactor);
	
	if(enableLightReflection) {
		vec4 normalColor = texture(normalMap, distortedTexCoords);
		vec3 normal = vec3(normalColor.r * 2.0 - 1.0, normalColor.b, normalColor.g * 2.0 - 1.0);
		normal = normalize(normal);
	
		vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
		vec3 reflectedLight = reflect(normalize(fromLightVector), normal);
		float specular = max(dot(reflectedLight, viewVector), 0.0);
		specular = pow(specular, shineDamper);
		vec3 specularHighlights = lightColor * specular * reflectivity;

		out_Color = mix(out_Color, vec4(0.0, 0.3, 0.5, 1.0), 0.2) + vec4(specularHighlights, 0.0);
	}

}

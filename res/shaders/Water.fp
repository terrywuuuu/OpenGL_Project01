#version 430 core

in vec4 clipSpace;
in vec2 textureCoords;

out vec4 out_Color;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D dudvMap;

uniform float moveFactor;

const float wateStrength = 0.01;

void main(void) {
	vec2 ndc = (clipSpace.xy / clipSpace.w) / 2 + 0.5;
	vec2 reflectionCoords = vec2(ndc.x, -ndc.y);
	vec2 refractionCoords = vec2(ndc.x, ndc.y);

	vec2 distortional = texture(dudvMap, vec2(textureCoords.x + moveFactor, textureCoords.y)).rg * 2.0 - 1.0;
	distortional *= wateStrength;

	reflectionCoords += distortional;
	reflectionCoords.x = clamp(reflectionCoords.x, 0.001, 0.999);
	reflectionCoords.y = clamp(reflectionCoords.y, -0.999, -0.001);
	refractionCoords += distortional;
	refractionCoords = clamp(refractionCoords, 0.001, 0.999);
	
	vec4 reflectionColor = texture(reflectionTexture, reflectionCoords);
	vec4 refractionColor = texture(refractionTexture, refractionCoords);
	
	out_Color = mix(reflectionColor, refractionColor, 0.5);
}

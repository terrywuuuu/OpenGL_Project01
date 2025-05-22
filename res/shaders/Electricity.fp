#version 430 core

in vec2 textureCoords;

out vec4 out_Color;

uniform sampler2D ElectricityTexture;
uniform sampler2D ElectricityBallTexture;
uniform int isBall;

void main(void) {
	if(isBall > 0) {
		vec4 ElectricityBallColors = texture(ElectricityBallTexture, textureCoords);
		out_Color = ElectricityBallColors;
	}else {
		vec4 ElectricityColors = texture(ElectricityTexture, textureCoords);
		out_Color = ElectricityColors;
	}
}

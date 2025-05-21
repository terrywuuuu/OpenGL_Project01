// ADS Point lighting Shader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 430

in vec2 TexCoords;
out vec4 FragColor;

uniform bool enableSmoke;    // 用來控制是否啟用煙霧
uniform bool enableFireWorks;    // 用來控制是否啟用煙火
uniform bool enableTrail;    // 用來控制是否啟用煙火尾焰       

uniform sampler2D effectTexture;
uniform float alpha;

in vec2 gl_PointCoord; 
in vec3 color;

void main(void)
{ 
    if(enableSmoke){
        FragColor = texture(effectTexture, TexCoords);
        FragColor.a *= alpha;
    }
    else if (enableFireWorks){
        vec4 texColor = texture(effectTexture, gl_PointCoord);
        FragColor = vec4(texColor.rgb, texColor.a * alpha);
    }
    else if (enableTrail){
        FragColor = vec4(color, 0.8);
    }
}

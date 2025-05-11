// ADS Point lighting Shader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 430

in vec2 TexCoords;
out vec4 FragColor;

uniform bool enableBlur;    // 用來控制是否啟用模糊
uniform bool enableQuan;    // 用來控制是否啟用量化
uniform bool enableMosaic;     // 控制是否啟用馬賽克
uniform bool enableMotionBlur;

uniform sampler2D sceneTexture;
uniform sampler2D motionTexture;
uniform vec2 texSize;            // 紋理的大小，用於計算偏移量

uniform float blurStrength;     // 模糊程度
uniform float quanStrength;     // 量化程度
uniform float mosaicSize;      // 馬賽克每格的像素大小 (程度)
uniform float motionBlurStrength;

// 計算模糊效果
vec3 applyBlur() {
    vec2 texelSize = blurStrength / texSize; // 每一個像素大小
    vec3 result = vec3(0.0);

    // 3x3 簡單模糊
    for(int x = -2; x <= 2; x++) {
        for(int y = -2; y <= 2; y++) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(sceneTexture, TexCoords + offset).rgb;
        }
    }

    result /= 25.0; // 取平均

    return result;
}

vec3 applyMosaic() {
    vec2 blockSize = vec2(mosaicSize) / texSize;
    vec2 mosaicUV = floor(TexCoords / blockSize) * blockSize + blockSize * 0.5;
    return texture(sceneTexture, mosaicUV).rgb;
}

void main(void)
{ 
    if(enableMotionBlur) {
        vec2 MotionVector = texture(motionTexture, TexCoords).xy / 3.0;

        vec4 Color = vec4(0.0);

        vec2 TexCoord = TexCoords;

        Color += texture(sceneTexture, TexCoord) * 0.4;
        TexCoord -= MotionVector;
        Color += texture(sceneTexture, TexCoord) * 0.3;
        TexCoord -= MotionVector;
        Color += texture(sceneTexture, TexCoord) * 0.2;
        TexCoord -= MotionVector;
        Color += texture(sceneTexture, TexCoord) * 0.1;

        FragColor = Color;
    }else if (enableBlur) {
        vec3 results = applyBlur();
        FragColor = vec4(results, 1.0);           // 應用模糊效果
    }
    else if(enableQuan){
        float nbins = quanStrength;
        vec4 tex_color = texture(sceneTexture, TexCoords);
        float r = floor(tex_color.r * nbins) / nbins;
        float g = floor(tex_color.g * nbins) / nbins;
        float b = floor(tex_color.b * nbins) / nbins;
        FragColor = vec4(r,g,b,tex_color.a);           // 量化效果
    }
    else if(enableMosaic){
        vec3 results = applyMosaic();
        FragColor = vec4(results, 1.0);           // 應用馬賽克效果
    }
    else{
        FragColor = texture(sceneTexture, TexCoords);   
    }
}

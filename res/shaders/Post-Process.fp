// ADS Point lighting Shader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 430

in vec2 TexCoords;
out vec4 FragColor;

uniform bool enableBlur;    // 用來控制是否啟用模糊
uniform sampler2D sceneTexture;  // 用來存儲渲染結果的紋理
uniform vec2 texSize;            // 紋理的大小，用於計算偏移量
uniform float blurStrength;     // 模糊程度

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

void main(void)
{ 
    // 如果啟用了模糊效果，則進行模糊處理
    if (enableBlur) {
        vec3 results = applyBlur();
        FragColor = vec4(results, 1.0);           // 應用模糊效果
    }
    else{
        FragColor = texture(sceneTexture, TexCoords);
    }
}

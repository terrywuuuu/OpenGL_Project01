#version 430

in vec3 v_worldPos;
out vec4 FragColor;

uniform float time;
uniform vec2 resolution;  

// 定義一些常數
#define pi 3.14159265
#define R(p, a) p = cos(a) * p + sin(a) * vec2(p.y, -p.x)
#define hsv(h, s, v) mix(vec3(1.0), clamp(abs(fract(h + vec3(3.0, 2.0, 1.0) / 3.0) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s) * v

// 生成噪聲
float hash(vec3 p) {
    return fract(sin(dot(p, vec3(17.0, 58.0, 113.0))) * 43758.5453);
}

float noise(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(mix(hash(i + vec3(0.0, 0.0, 0.0)), hash(i + vec3(1.0, 0.0, 0.0)), f.x),
                   mix(hash(i + vec3(0.0, 1.0, 0.0)), hash(i + vec3(1.0, 1.0, 0.0)), f.x), f.y),
               mix(mix(hash(i + vec3(0.0, 0.0, 1.0)), hash(i + vec3(1.0, 0.0, 1.0)), f.x),
                   mix(hash(i + vec3(0.0, 1.0, 1.0)), hash(i + vec3(1.0, 1.0, 1.0)), f.x), f.y), f.z);
}

// 計算火焰形狀
float flameShape(vec3 p) {
    p.y += time * 0.8;
    float base = length(p * vec3(1.0, 1.3, 1.0)) - 1.0;

    float t1 = noise(p * 3.0 + vec3(0.0, time * 2.0, 0.0)) * 0.3;
    float t2 = noise(p * 6.0 + vec3(10.0, time * 4.0, 0.0)) * 0.2;

    return base + t1 + t2;
}

void main() {
    float d = flameShape(v_worldPos);
    d = clamp(d, 0.0, 1.0); 

    float glow = exp(-0.5 * d);
    float hue = 0.05 + 0.1 * d + 0.05 * sin(time * 5.0);
    vec3 color = hsv(hue, 1.0, 1.0) * glow;

    FragColor = vec4(color, glow);
}
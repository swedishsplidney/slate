#version 450

layout(location = 0) in vec3 fragWorldPos;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUBO {
    vec3 cameraPos;
    vec3 lightDirection;
    vec3 lightColor;
    float lightIntensity;
} ubo;

float grid(vec2 coord, float targetPixelWidth, out vec2 outPixelDist) {
    vec2 grid = abs(fract(coord - 0.5) - 0.5);
    vec2 fw = fwidth(coord);
    fw = max(fw, vec2(0.00001));
    outPixelDist = grid / fw;
    float minDist = min(outPixelDist.x, outPixelDist.y);
    return 1.0 - smoothstep(0.0, targetPixelWidth, minDist);
}

void main() {
    vec2 pos = fragWorldPos.xz;
    float targetPixels = 1.2;

    vec2 minorPixelDist;
    float minorGrid = grid(pos, targetPixels, minorPixelDist);
    float minDistMinor = min(minorPixelDist.x, minorPixelDist.y);

    vec2 majorPixelDist;
    float majorGrid = grid(pos * 0.1, targetPixels, majorPixelDist);
    float minDistMajor = min(majorPixelDist.x, majorPixelDist.y);

    float dist = distance(fragWorldPos, ubo.cameraPos);
    float distFade = 1.0 - smoothstep(90.0, 120.0, dist);

    float pattern = max(minorGrid * 0.35, majorGrid);

    if (pattern < 0.01) {
        discard;
    }

    vec3 lineColor = mix(vec3(0.2, 0.21, 0.24), vec3(0.35, 0.38, 0.45), majorGrid);
    
    vec2 minorCell = round(pos);
    vec2 majorCell = round(pos * 0.1);
    
    bool isZAxis = (minorCell.x == 0.0 && minDistMinor == minorPixelDist.x && minorGrid > 0.01) ||
    (majorCell.x == 0.0 && minDistMajor == majorPixelDist.x && majorGrid > 0.01);

    bool isXAxis = (minorCell.y == 0.0 && minDistMinor == minorPixelDist.y && minorGrid > 0.01) ||
    (majorCell.y == 0.0 && minDistMajor == majorPixelDist.y && majorGrid > 0.01);

    if (isZAxis) {
        lineColor = vec3(0.85, 0.22, 0.22);
    }
    if (isXAxis) {
        lineColor = vec3(0.22, 0.40, 0.85);
    }

    float finalAlpha = pattern * distFade;
    if (finalAlpha <= 0.001) {
        discard;
    }

    outColor = vec4(lineColor, finalAlpha);
}
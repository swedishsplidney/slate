#version 450

layout(set = 0, binding = 0) uniform sampler2D fontTexture;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

void main() {
    if (fragUV.x < 0.0 || fragUV.y < 0.0) {
        outColor = fragColor;
    } else {
        float alpha = texture(fontTexture, fragUV).a;
        outColor = vec4(fragColor.rgb, fragColor.a * alpha);
    }
}
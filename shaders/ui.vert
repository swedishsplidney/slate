#version 450

layout(push_constant) uniform PushConstants {
    mat4 orthoProj;
} push;

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 fragColor;

void main() {
    gl_Position = push.orthoProj * vec4(inPos, 0.0, 1.0);
    fragColor = inColor;
}
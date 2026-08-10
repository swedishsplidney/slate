#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragWorldPos;

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;
    mat4 viewProjMatrix;
} pc;

void main() {
    vec4 worldPos = pc.modelMatrix * vec4(inPosition, 1.0);
    fragWorldPos = worldPos.xyz;
    gl_Position = pc.viewProjMatrix * worldPos;
}
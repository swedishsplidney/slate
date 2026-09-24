#version 450

layout(location = 0) in vec3 inPos;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) flat out uint fragMaterialIndex;

layout(std140, set = 0, binding = 0) uniform GlobalUBO {
    vec3  cameraPos;
    float exposure;
    vec3  lightDirection;
    float _pad0;
    vec3  lightColor;
    float lightIntensity;
    vec4  ambientCube[6];
    mat4  lightSpaceMatrix;
} ubo;

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;
    mat4 viewProjMatrix;
    uint materialId;
} push;

void main() {
    fragTexCoord = inTexCoord;
    fragMaterialIndex = push.materialId;
    gl_Position = ubo.lightSpaceMatrix * push.modelMatrix * vec4(inPos, 1.0);
}
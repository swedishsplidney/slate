#version 450

layout(location = 0) in vec3 inPos;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) flat out uint fragMaterialIndex;

// std140
layout(std140, set = 0, binding = 0) uniform GlobalUBO {
    vec3  cameraPos;
    float exposure;

    vec3  lightDirection;
    int   lightType;

    vec3  lightColor;
    float lightIntensity;

    vec3  lightPos;
    float lightRange;

    vec4  lightParams;

    vec4  ambientCube[6];
    mat4  lightSpaceMatrix;

    mat4  shadowMatrices[6];
    vec4  cameraForward;
    vec4  cascadeSplits;
    vec4  cascadeTexelWorld;
    vec4  cascadeDepthRange;
    vec4  shadowParams0;
    vec4  shadowParams1;
    vec4  shadowParams2;
    vec4  shadowParams3;
    vec4  shadowParams4;
} ubo;

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;
    uint materialId;
    uint layer;
} push;

void main() {
    fragTexCoord = inTexCoord;
    fragMaterialIndex = push.materialId;
    gl_Position = ubo.shadowMatrices[push.layer] * push.modelMatrix * vec4(inPos, 1.0);
}

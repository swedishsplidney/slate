#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragPosWorld;
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec2 fragTexCoord;
layout(location = 4) flat out uint fragMaterialIndex;
layout(location = 5) out vec4 fragPosLightSpace;

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
} ubo;

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;
    mat4 viewProjMatrix;
    uint materialId;
} push;

void main() {
    vec4 worldPos = push.modelMatrix * vec4(inPos, 1.0);
    mat3 normalMatrix = mat3(transpose(inverse(push.modelMatrix)));
    vec3 worldNormal = normalize(normalMatrix * inNormal);

    fragPosWorld = worldPos.xyz;
    fragColor = inColor;
    fragNormal = normalMatrix * inNormal;
    fragTexCoord = inTexCoord;
    fragMaterialIndex = push.materialId;

    vec3 shadowSamplePos = worldPos.xyz + worldNormal * ubo.lightParams.y;
    fragPosLightSpace = ubo.lightSpaceMatrix * vec4(shadowSamplePos, 1.0);

    gl_Position = push.viewProjMatrix * worldPos;
}
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

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;
    mat4 viewProjMatrix;
    uint materialId;
} push;

void main() {
    vec4 worldPos = push.modelMatrix * vec4(inPos, 1.0);
    fragPosWorld = worldPos.xyz;
    fragColor = inColor;

    mat3 normalMatrix = transpose(inverse(mat3(push.modelMatrix)));
    fragNormal = normalize(normalMatrix * inNormal);

    fragTexCoord = inTexCoord;
    fragMaterialIndex = push.materialId;

    gl_Position = push.viewProjMatrix * worldPos;
}
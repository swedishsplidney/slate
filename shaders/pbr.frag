#version 450

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragTexCoord;
layout(location = 4) flat in uint fragMaterialIndex;

layout(location = 0) out vec4 outColor;

struct MaterialGPU {
    vec4 albedoFactor;
    float roughnessFactor;
    float metallicFactor;
    vec2 padding;
};

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;
    mat4 viewProjMatrix;
    uint materialId;
} push;

// descriptor sets
layout(std140, set = 0, binding = 0) uniform GlobalUBO {
    vec3 cameraPos;
    vec3 lightDirection;
    vec3 lightColor;
    float lightIntensity;
} ubo;

layout(std430, set = 1, binding = 0) readonly buffer MaterialBuffer {
    MaterialGPU materials[];
} materialBuffer;

const float PI = 3.14159265359;

// pbr cook torrence stuff

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / max(denom, 0.000001);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    uint activeMaterialId = push.materialId != 0 ? push.materialId : fragMaterialIndex;
    MaterialGPU mat = materialBuffer.materials[push.materialId];

    vec3 N = normalize(fragNormal);
    vec3 V = normalize(ubo.cameraPos - fragPosWorld);
    vec3 L = normalize(ubo.lightDirection);
    vec3 H = normalize(V + L);

    vec3 albedo     = mat.albedoFactor.rgb * fragColor;
    float roughness = clamp(mat.roughnessFactor, 0.05, 1.0);
    float metallic  = clamp(mat.metallicFactor, 0.0, 1.0);
    float ao        = 1.0;

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 F   = fresnelSchlick(max(dot(H, V), 0.0), F0);
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    float NdotL   = max(dot(N, L), 0.0);
    vec3 radiance = ubo.lightColor * ubo.lightIntensity;

    vec3 Lo      = (kD * albedo / PI + specular) * radiance * NdotL;
    vec3 ambient = vec3(0.10) * albedo * ao;

    vec3 color = ambient + Lo;

    // tone mapping
    color = color / (color + vec3(1.0));

    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, 1.0);
}
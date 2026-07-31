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
    float transmissionFactor;
    float ior;
    vec4 padding;
};

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;
    mat4 viewProjMatrix;
    uint materialId;
} push;

layout(std140, set = 0, binding = 0) uniform GlobalUBO {
    vec3 cameraPos;
    vec3 lightDirection;
    vec3 lightColor;
    float lightIntensity;
} ubo;

layout(set = 0, binding = 1) uniform sampler2D sceneColorTexture;

layout(std430, set = 1, binding = 0) readonly buffer MaterialBuffer {
    MaterialGPU materials[];
} materialBuffer;

const float PI = 3.14159265359;

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
    uint activeMaterialId = push.materialId;
    MaterialGPU mat = materialBuffer.materials[activeMaterialId];

    vec3 baseN = normalize(fragNormal);
    float transmission = clamp(mat.transmissionFactor, 0.0, 1.0);
    float baseAlpha = clamp(mat.albedoFactor.a, 0.0, 1.0);

    float ior = mat.ior <= 1.0 ? 1.5 : mat.ior;

    vec3 V = normalize(ubo.cameraPos - fragPosWorld);
    vec3 N = baseN;

    // Flip normal if rendering backfaces
    if (!gl_FrontFacing) {
        N = -N;
    }

    vec3 L = normalize(ubo.lightDirection);
    vec3 H = normalize(V + L);

    vec3 vColor = length(fragColor) > 0.001 ? fragColor : vec3(1.0);
    vec3 albedo = mat.albedoFactor.rgb * vColor;

    float roughness = clamp(mat.roughnessFactor, 0.08, 1.0);
    float metallic  = clamp(mat.metallicFactor, 0.0, 1.0);
    float ao        = 1.0;

    float iorF0 = pow((ior - 1.0) / (ior + 1.0), 2.0);
    vec3 F0 = mix(vec3(iorF0), albedo, metallic);

    float NdotV = max(dot(N, V), 0.0);
    vec3 F   = fresnelSchlick(NdotV, F0);
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * NdotV * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    float NdotL   = max(dot(N, L), 0.0);
    vec3 radiance = ubo.lightColor * ubo.lightIntensity;

    vec3 Lo      = (kD * albedo / PI + specular) * radiance * NdotL;
    vec3 ambient = vec3(0.15) * albedo * ao;

    vec3 color = ambient + Lo;

    float viewAngle = max(dot(N, V), 0.0);
    float fresnelRim = pow(1.0 - viewAngle, 4.0);

    if (transmission > 0.01) {
        ivec2 texSize = textureSize(sceneColorTexture, 0);
        if (texSize.x > 0 && texSize.y > 0) {
            vec2 screenUV = gl_FragCoord.xy / vec2(texSize);

            vec3 refractDir = refract(-V, N, 1.0 / ior);
            if (length(refractDir) < 0.001) {
                refractDir = reflect(-V, N);
            }

            vec3 rayDeviation = refractDir - (-V);
            vec2 distortion = (push.viewProjMatrix * vec4(rayDeviation, 0.0)).xy * 0.05 * transmission;

            vec2 refractUV = clamp(screenUV + distortion, vec2(0.001), vec2(0.999));

            vec3 backgroundScene = texture(sceneColorTexture, refractUV).rgb;

            vec3 refractedColor = (backgroundScene * albedo) + specular + (fresnelRim * albedo * 0.3);
            color = mix(color, refractedColor, transmission);
        }
    } else {
        color = color / (color + vec3(1.0));
        color = pow(color, vec3(1.0 / 2.2));

        outColor = vec4(color, 1.0);
        return;
    }

    // tone mapping
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    float fresnelAlpha = mix(baseAlpha * 0.2, baseAlpha, pow(1.0 - viewAngle, 3.0));
    float outAlpha = mix(fresnelAlpha, 1.0, transmission);

    outColor = vec4(color, outAlpha);
}
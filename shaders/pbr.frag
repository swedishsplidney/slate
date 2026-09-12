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
    float aoFactor;
    int hasAlbedoTexture;
    int hasNormalTexture;
    int hasOrmTexture;
    float padding[1];
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

layout(set = 1, binding = 1) uniform sampler2D albedoMap;
layout(set = 1, binding = 2) uniform sampler2D normalMap;
layout(set = 1, binding = 3) uniform sampler2D ormMap;

layout(std430, set = 1, binding = 0) readonly buffer MaterialBuffer {
    MaterialGPU materials[];
} materialBuffer;

const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (vec3(1.0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

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
    return num / max(denom, 0.0001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 computeDisneyDiffuse(vec3 albedo, float roughness, float NdotV, float NdotL, float LdotH) {
    float energyBias = mix(0.0, 0.5, roughness);
    float energyFactor = mix(1.0, 1.0 / 1.51, roughness);
    float FD90 = energyBias + 2.0 * LdotH * LdotH * roughness;
    float lightScatter = 1.0 + (FD90 - 1.0) * pow(clamp(1.0 - NdotL, 0.0, 1.0), 5.0);
    float viewScatter  = 1.0 + (FD90 - 1.0) * pow(clamp(1.0 - NdotV, 0.0, 1.0), 5.0);
    return albedo * (lightScatter * viewScatter * energyFactor / PI);
}

vec3 toneMapPBRNeutral(vec3 color) {
    const float startCompression = 0.8 - 0.04;
    const float desaturation = 0.15;
    float x = min(color.r, min(color.g, color.b));
    float offset = x < 0.08 ? x - 6.25 * x * x : 0.04;
    color -= offset;
    float peak = max(color.r, max(color.g, color.b));
    if (peak < startCompression) return color;
    const float d = 1.0 - startCompression;
    float newPeak = 1.0 - d * d / (peak + d - startCompression);
    color *= newPeak / peak;
    float g = 1.0 - 1.0 / (desaturation * (peak - newPeak) + 1.0);
    return mix(color, vec3(newPeak), g);
}

void main() {
    uint activeMaterialId = fragMaterialIndex;
    MaterialGPU mat = materialBuffer.materials[activeMaterialId];

    vec3 N = normalize(fragNormal);
    if (!gl_FrontFacing) N = -N;

    vec3 V = normalize(ubo.cameraPos - fragPosWorld);
    vec3 R = reflect(-V, N);

    float roughness = clamp(mat.roughnessFactor, 0.04, 1.0);
    float metallic  = clamp(mat.metallicFactor, 0.0, 1.0);

    float transmission = clamp(mat.transmissionFactor, 0.0, 1.0) * (1.0 - metallic);

    float baseAlpha = clamp(mat.albedoFactor.a, 0.0, 1.0);
    float ior = mat.ior <= 1.0 ? 1.5 : mat.ior;
    float ao = clamp(mat.aoFactor > 0.0 ? mat.aoFactor : 1.0, 0.0, 1.0);

    vec3 vColor = length(fragColor) > 0.001 ? fragColor : vec3(1.0);

    // albedo
    vec4 baseAlbedo = mat.albedoFactor;
    if (mat.hasAlbedoTexture > 0) {
        baseAlbedo = texture(albedoMap, fragTexCoord);
    }
    vec3 albedo = baseAlbedo.rgb * (length(fragColor) > 0.001 ? fragColor : vec3(1.0));

    // orm
    roughness = mat.roughnessFactor;
    metallic  = mat.metallicFactor;
    ao        = mat.aoFactor;

    if (mat.hasOrmTexture > 0) {
        vec3 ormSample = texture(ormMap, fragTexCoord).rgb;
        ao        *= ormSample.r;
        roughness *= ormSample.g;
        metallic  *= ormSample.b;
    }

    roughness = clamp(roughness, 0.04, 1.0);
    metallic  = clamp(metallic, 0.0, 1.0);
    ao        = clamp(ao, 0.0, 1.0);

    // normal
    N = normalize(fragNormal);
    if (!gl_FrontFacing) N = -N;

    if (mat.hasNormalTexture > 0) {
        vec3 tangentNormal = texture(normalMap, fragTexCoord).rgb * 2.0 - 1.0;

        vec3 Q1  = dFdx(fragPosWorld);
        vec3 Q2  = dFdy(fragPosWorld);
        vec2 st1 = dFdx(fragTexCoord);
        vec2 st2 = dFdy(fragTexCoord);

        vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
        vec3 B = normalize(cross(N, T));
        mat3 TBN = mat3(T, B, N);

        N = normalize(TBN * tangentNormal);
    }

    baseAlpha = clamp(baseAlbedo.a, 0.0, 1.0);

    float iorF0 = pow((ior - 1.0) / (ior + 1.0), 2.0);
    vec3 dielectricF0 = vec3(iorF0);
    vec3 F0 = mix(dielectricF0, albedo, metallic);

    float NdotV = max(dot(N, V), 0.0001);

    // direct
    vec3 L = length(ubo.lightDirection) > 0.1 ? normalize(ubo.lightDirection) : normalize(vec3(0.5, 1.0, 0.3));
    vec3 lightColor = length(ubo.lightColor) > 0.1 ? ubo.lightColor * ubo.lightIntensity : vec3(1.0, 0.95, 0.9) * 2.5;

    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    float LdotH = max(dot(L, H), 0.0);

    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 specularNumerator   = NDF * G * F;
    float specularDenominator = 4.0 * NdotV * NdotL + 0.0001;
    vec3 specularDirect       = specularNumerator / specularDenominator;

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    vec3 diffuseDirect = computeDisneyDiffuse(albedo, roughness, NdotV, NdotL, LdotH);
    vec3 directLight = (kD * diffuseDirect + specularDirect) * lightColor * NdotL;

    vec3 skyColor = vec3(0.5, 0.5, 0.5);
    vec3 groundColor = vec3(0.15, 0.15, 0.15);

    vec3 irradiance = mix(groundColor, skyColor, N.y * 0.5 + 0.5);
    vec3 blurredR = normalize(mix(R, N, roughness * 0.7));
    vec3 radiance = mix(groundColor, skyColor, blurredR.y * 0.5 + 0.5);

    vec3 F_ambient = fresnelSchlickRoughness(NdotV, F0, roughness);
    vec3 kS_amb = F_ambient;
    vec3 kD_amb = (vec3(1.0) - kS_amb) * (1.0 - metallic);

    vec3 ambientDiffuse = kD_amb * albedo * irradiance * ao;
    vec3 ambientSpecular = radiance * F_ambient * ao;

    vec3 ambientLight = ambientDiffuse + ambientSpecular;
    vec3 color = ambientLight + directLight;

    // transmission
    if (transmission > 0.01) {
        ivec2 texSize = textureSize(sceneColorTexture, 0);
        if (texSize.x > 0 && texSize.y > 0) {
            vec2 screenUV = gl_FragCoord.xy / vec2(texSize);

            vec3 refractDir = refract(-V, N, 1.0 / ior);
            if (length(refractDir) < 0.001) refractDir = reflect(-V, N);

            vec3 rayDeviation = refractDir - (-V);
            vec2 distortion = (push.viewProjMatrix * vec4(rayDeviation, 0.0)).xy * 0.05 * transmission;
            vec2 refractUV = clamp(screenUV + distortion, vec2(0.001), vec2(0.999));

            vec3 backgroundScene = texture(sceneColorTexture, refractUV).rgb;
            vec3 glassSpecular = (specularDirect * lightColor * NdotL) + ambientSpecular;

            vec3 transmittedLight = backgroundScene * albedo * (vec3(1.0) - F_ambient);
            vec3 refractedColor = transmittedLight + glassSpecular;

            color = mix(color, refractedColor, transmission);
        }
    }

    // tone mapping
    color = toneMapPBRNeutral(color);

    float fresnelAlpha = mix(baseAlpha * 0.2, baseAlpha, pow(1.0 - NdotV, 3.5));
    float nonMetalAlpha = mix(fresnelAlpha, 1.0, transmission);
    float outAlpha = mix(nonMetalAlpha, 1.0, metallic);

    outColor = vec4(color, outAlpha);
}
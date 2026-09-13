#version 450

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragTexCoord;
layout(location = 4) flat in uint fragMaterialIndex;

layout(location = 0) out vec4 outColor;

struct MaterialGPU {
    vec4 albedoFactor;
    vec4 emissiveFactor;

    float roughnessFactor;
    float metallicFactor;
    float transmissionFactor;
    float ior;

    float aoFactor;
    float rimIntensity;
    float rimExponent;
    float alphaCutoff;

    int hasAlbedoTexture;
    int hasNormalTexture;
    int hasOrmTexture;
    int hasEmissiveTexture;
};

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;
    mat4 viewProjMatrix;
    uint materialId;
} push;

layout(std140, set = 0, binding = 0) uniform GlobalUBO {
    vec3  cameraPos;
    float exposure;
    vec3  lightDirection;
    float _pad0;
    vec3  lightColor;
    float lightIntensity;
    vec4  ambientCube[6];
} ubo;

layout(set = 0, binding = 1) uniform sampler2D sceneColorTexture;
layout(set = 1, binding = 1) uniform sampler2D albedoMap;
layout(set = 1, binding = 2) uniform sampler2D normalMap;
layout(set = 1, binding = 3) uniform sampler2D ormMap;
layout(set = 1, binding = 4) uniform sampler2D emissiveMap;

layout(std430, set = 1, binding = 0) readonly buffer MaterialBuffer {
    MaterialGPU materials[];
} materialBuffer;

const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (vec3(1.0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 envBRDFApprox(vec3 F0, float roughness, float NoV) {
    const vec4 c0 = vec4(-1.0, -0.0275, -0.572, 0.022);
    const vec4 c1 = vec4(1.0, 0.0425, 1.04, -0.04);
    vec4 r = roughness * c0 + c1;
    float a004 = min(r.x * r.x, exp2(-9.28 * NoV)) * r.x + r.y;
    vec2 AB = vec2(-1.04, 0.04) * a004 + r.zw;
    return F0 * AB.x + vec3(AB.y);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return num / max(PI * denom * denom, 0.000001);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / max(NdotV * (1.0 - k) + k, 0.0001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness) *
    GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}

vec3 computeDisneyDiffuse(vec3 albedo, float roughness, float NdotV, float NdotL, float LdotH) {
    float energyBias = mix(0.0, 0.5, roughness);
    float energyFactor = mix(1.0, 1.0 / 1.51, roughness);
    float FD90 = energyBias + 2.0 * LdotH * LdotH * roughness;
    float lightScatter = 1.0 + (FD90 - 1.0) * pow(clamp(1.0 - NdotL, 0.0, 1.0), 5.0);
    float viewScatter  = 1.0 + (FD90 - 1.0) * pow(clamp(1.0 - NdotV, 0.0, 1.0), 5.0);
    return albedo * (lightScatter * viewScatter * energyFactor / PI);
}

// ambient cube
vec3 sampleAmbientCube(vec3 n) {
    vec3 n2 = n * n;
    vec3 xColor = mix(ubo.ambientCube[1].rgb, ubo.ambientCube[0].rgb, step(0.0, n.x));
    vec3 yColor = mix(ubo.ambientCube[3].rgb, ubo.ambientCube[2].rgb, step(0.0, n.y));
    vec3 zColor = mix(ubo.ambientCube[5].rgb, ubo.ambientCube[4].rgb, step(0.0, n.z));
    return n2.x * xColor + n2.y * yColor + n2.z * zColor;
}

// normal
mat3 cotangentFrame(vec3 N, vec3 p, vec2 uv) {
    vec3 dp1 = dFdx(p);
    vec3 dp2 = dFdy(p);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
    return mat3(T * invmax, B * invmax, N);
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
    MaterialGPU mat = materialBuffer.materials[fragMaterialIndex];

    vec3 N_base = normalize(fragNormal);
    if (!gl_FrontFacing) N_base = -N_base;

    vec3 V = normalize(ubo.cameraPos - fragPosWorld);

    // albedo
    vec4 baseAlbedo = (mat.hasAlbedoTexture > 0) ? texture(albedoMap, fragTexCoord) : mat.albedoFactor;
    vec3 vertexTint = (length(fragColor) > 0.001) ? fragColor : vec3(1.0);
    vec3 albedo = baseAlbedo.rgb * vertexTint;
    float baseAlpha = clamp(baseAlbedo.a, 0.0, 1.0);

    // alpha test
    if (mat.alphaCutoff > 0.0 && baseAlpha < mat.alphaCutoff) {
        discard;
    }

    // orm
    float roughness = mat.roughnessFactor;
    float metallic  = mat.metallicFactor;
    float ao        = mat.aoFactor > 0.0 ? mat.aoFactor : 1.0;

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
    vec3 N = N_base;
    if (mat.hasNormalTexture > 0) {
        vec3 tangentNormal = texture(normalMap, fragTexCoord).rgb * 2.0 - 1.0;
        mat3 TBN = cotangentFrame(N_base, fragPosWorld, fragTexCoord);
        N = normalize(TBN * tangentNormal);
    }

    // specular aa
    float normalVariance = length(dFdx(N)) + length(dFdy(N));
    float geometricRoughness = clamp(normalVariance * 0.4, 0.0, 0.3);
    float effectiveRoughness = clamp(sqrt(roughness * roughness + geometricRoughness), 0.04, 1.0);

    float ior = mat.ior <= 1.0 ? 1.5 : mat.ior;
    float transmission = clamp(mat.transmissionFactor, 0.0, 1.0) * (1.0 - metallic);

    float iorF0 = pow((ior - 1.0) / (ior + 1.0), 2.0);
    vec3 F0 = mix(vec3(iorF0), albedo, metallic);

    float NdotV = max(dot(N, V), 0.0001);
    vec3 R = reflect(-V, N);

    // direct lighting
    vec3 L = length(ubo.lightDirection) > 0.1 ? normalize(ubo.lightDirection) : normalize(vec3(0.5, 1.0, 0.3));
    vec3 lightColor = length(ubo.lightColor) > 0.1 ? ubo.lightColor * ubo.lightIntensity : vec3(1.0, 0.95, 0.9) * 2.5;

    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    float LdotH = max(dot(L, H), 0.0);

    float NDF = DistributionGGX(N, H, effectiveRoughness);
    float G   = GeometrySmith(N, V, L, effectiveRoughness);
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 specularDirect = (NDF * G * F) / max(4.0 * NdotV * NdotL + 0.0001, 0.0001);
    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    vec3 diffuseDirect = computeDisneyDiffuse(albedo, effectiveRoughness, NdotV, NdotL, LdotH);
    vec3 directLight = (kD * diffuseDirect + specularDirect) * lightColor * NdotL;

    // ambient
    vec3 irradiance = sampleAmbientCube(N);
    vec3 blurredR = normalize(mix(R, N, effectiveRoughness * 0.6));
    vec3 radiance = sampleAmbientCube(blurredR);

    vec3 F_env = envBRDFApprox(F0, effectiveRoughness, NdotV);
    vec3 kD_amb = (vec3(1.0) - F_env) * (1.0 - metallic);

    // specular occlusion
    float specOcclusion = clamp(pow(NdotV + ao, exp2(-16.0 * effectiveRoughness - 1.0)) - 1.0 + ao, 0.0, 1.0);

    vec3 ambientDiffuse = kD_amb * albedo * irradiance * ao;
    vec3 ambientSpecular = radiance * F_env * specOcclusion;

    vec3 color = ambientDiffuse + ambientSpecular + directLight;

    // rim light
    if (mat.rimIntensity > 0.0) {
        float rimExp = max(mat.rimExponent, 0.5);
        float rimFactor = pow(clamp(1.0 - NdotV, 0.0, 1.0), rimExp);
        color += mat.rimIntensity * rimFactor * mix(irradiance, lightColor, 0.5) * ao;
    }

    // emissive
    vec3 emissive = mat.emissiveFactor.rgb * mat.emissiveFactor.a;
    if (mat.hasEmissiveTexture > 0) {
        emissive *= texture(emissiveMap, fragTexCoord).rgb;
    }
    color += emissive;

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

            vec3 transmittedLight = backgroundScene * albedo * (vec3(1.0) - F_env);
            color = mix(color, transmittedLight + glassSpecular + emissive, transmission);
        }
    }

    color *= ubo.exposure > 0.0 ? ubo.exposure : 1.0;
    color = toneMapPBRNeutral(color);

    float glassEdgeAlpha = mix(baseAlpha * 0.2, baseAlpha, pow(1.0 - NdotV, 3.5));
    float dielectricAlpha = mix(baseAlpha, glassEdgeAlpha, transmission);
    outColor = vec4(color, mix(dielectricAlpha, 1.0, metallic));
}

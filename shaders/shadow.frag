#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) flat in uint fragMaterialIndex;

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

    vec4 detailParams;
};

layout(set = 1, binding = 1) uniform sampler2D albedoMap;

layout(std430, set = 1, binding = 0) readonly buffer MaterialBuffer {
    MaterialGPU materials[];
} materialBuffer;

void main() {
    MaterialGPU mat = materialBuffer.materials[fragMaterialIndex];

    if (mat.alphaCutoff > 0.0) {
        vec4 baseAlbedo = (mat.hasAlbedoTexture > 0) ? texture(albedoMap, fragTexCoord) : mat.albedoFactor;
        if (baseAlbedo.a < mat.alphaCutoff) {
            discard;
        }
    }
}
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace slate {

    constexpr uint32_t MAX_SHADOW_CASCADES = 4;
    constexpr uint32_t SHADOW_CUBE_FACES   = 6;
    constexpr uint32_t SHADOW_MAP_LAYERS   = 6;

    enum class ShadowQuality : int32_t {
        Low    = 0,   // 4 taps
        Medium = 1,   // 8 taps
        High   = 2,   // 16 taps
        Ultra  = 3    // 32 taps
    };

    struct ShadowSettings {
        bool enabled = true;

        uint32_t resolution = 2048;
        ShadowQuality quality = ShadowQuality::High;

        bool contactHardening = true;

        uint32_t cascadeCount = 4;
        float maxDistance = 120.0f;
        float splitLambda = 0.75f;
        float cascadeBlend = 0.15f;
        float fadeFraction = 0.15f;
        float casterExtension = 200.0f;

        float normalBiasTexels = 1.5f;
        float depthBiasTexels  = 1.0f;
        float rasterDepthBias  = 1.0f;
        float rasterSlopeBias  = 1.0f;

        bool cullCasters = true;
        bool debugCascades = false;
    };

    inline ShadowSettings sanitizeShadowSettings(ShadowSettings s) {
        s.resolution       = std::clamp(s.resolution, 256u, 8192u);
        s.cascadeCount     = std::clamp(s.cascadeCount, 1u, MAX_SHADOW_CASCADES);
        s.maxDistance      = std::max(s.maxDistance, 1.0f);
        s.splitLambda      = std::clamp(s.splitLambda, 0.0f, 1.0f);
        s.cascadeBlend     = std::clamp(s.cascadeBlend, 0.0f, 0.5f);
        s.fadeFraction     = std::clamp(s.fadeFraction, 0.01f, 0.5f);
        s.casterExtension  = std::max(s.casterExtension, 0.0f);
        s.normalBiasTexels = std::max(s.normalBiasTexels, 0.0f);
        s.depthBiasTexels  = std::max(s.depthBiasTexels, 0.0f);
        return s;
    }

    struct ShadowQualityPreset {
        int pcfTaps;
        int blockerTaps;
        float maxFilterTexels;
    };

    inline ShadowQualityPreset getShadowQualityPreset(ShadowQuality q) {
        switch (q) {
            case ShadowQuality::Low:    return {4, 0, 2.0f};
            case ShadowQuality::Medium: return {8, 6, 6.0f};
            case ShadowQuality::High:   return {16, 10, 10.0f};
            case ShadowQuality::Ultra:  return {32, 16, 16.0f};
        }
        return {16, 10, 10.0f};
    }

    // cascaded shadow maps

    struct CascadeData {
        std::array<glm::mat4, MAX_SHADOW_CASCADES> viewProj{};
        std::array<float, MAX_SHADOW_CASCADES> splitFar{};
        std::array<float, MAX_SHADOW_CASCADES> texelWorld{};
        std::array<float, MAX_SHADOW_CASCADES> depthRange{};
        uint32_t count = 0;
    };

    inline std::array<float, MAX_SHADOW_CASCADES> computeCascadeSplits(float nearPlane, float farPlane,
                                                                      uint32_t count, float lambda) {
        std::array<float, MAX_SHADOW_CASCADES> splits{};
        count = std::clamp(count, 1u, MAX_SHADOW_CASCADES);
        nearPlane = std::max(nearPlane, 0.01f);
        farPlane = std::max(farPlane, nearPlane + 0.01f);

        for (uint32_t i = 0; i < MAX_SHADOW_CASCADES; ++i) {
            if (i >= count) {
                splits[i] = farPlane;
                continue;
            }
            const float p = static_cast<float>(i + 1) / static_cast<float>(count);
            const float logSplit = nearPlane * std::pow(farPlane / nearPlane, p);
            const float uniSplit = nearPlane + (farPlane - nearPlane) * p;
            splits[i] = lambda * logSplit + (1.0f - lambda) * uniSplit;
        }
        splits[count - 1] = farPlane;
        return splits;
    }

    inline void frustumSliceBoundingSphere(float fovY, float aspect, float n, float f,
                                           float& centerDepth, float& radius) {
        const float tanHalf = std::tan(fovY * 0.5f);
        const float k = std::sqrt(1.0f + aspect * aspect) * tanHalf;
        if (k * k >= (f - n) / (f + n)) {
            centerDepth = f;
            radius = f * k;
        } else {
            centerDepth = 0.5f * (f + n) * (1.0f + k * k);
            radius = 0.5f * std::sqrt((f - n) * (f - n)
                                      + 2.0f * (f * f + n * n) * k * k
                                      + (f + n) * (f + n) * k * k * k * k);
        }
    }

    inline CascadeData computeSunCascades(const glm::vec3& cameraPos, const glm::vec3& cameraForward,
                                          float fovY, float aspect, float cameraNear,
                                          const glm::vec3& sunDirToLight,
                                          const ShadowSettings& settings, float guardTexels) {
        CascadeData out{};
        const ShadowSettings s = sanitizeShadowSettings(settings);
        out.count = s.cascadeCount;

        const float shadowNear = std::max(cameraNear, 0.1f);
        const float shadowFar = std::max(s.maxDistance, shadowNear + 1.0f);
        out.splitFar = computeCascadeSplits(shadowNear, shadowFar, out.count, s.splitLambda);

        const glm::vec3 sunDir = glm::normalize(sunDirToLight);
        const glm::vec3 upHint = (std::abs(sunDir.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, 1.0f)
                                                               : glm::vec3(0.0f, 1.0f, 0.0f);

        const glm::mat4 lightRot = glm::lookAtRH(glm::vec3(0.0f), -sunDir, upHint);

        const float res = static_cast<float>(s.resolution);
        const float guard = std::max(guardTexels, 0.0f);

        for (uint32_t i = 0; i < out.count; ++i) {
            float sliceNear = cameraNear;
            if (i > 0) {
                const float prevEnd = out.splitFar[i - 1];
                const float prevStart = (i > 1) ? out.splitFar[i - 2] : 0.0f;
                sliceNear = std::max(prevEnd - s.cascadeBlend * (prevEnd - prevStart), cameraNear);
            }
            const float sliceFar = out.splitFar[i];

            float centerDepth = 0.0f;
            float radius = 0.0f;
            frustumSliceBoundingSphere(fovY, aspect, sliceNear, sliceFar, centerDepth, radius);
            radius = std::ceil(radius * 16.0f) / 16.0f;

            const float usableTexels = std::max(res - 2.0f * guard, 16.0f);
            const float texel = (2.0f * radius) / usableTexels;
            const float halfExtent = texel * res * 0.5f;

            const glm::vec3 center = cameraPos + cameraForward * centerDepth;
            glm::vec3 centerLS = glm::vec3(lightRot * glm::vec4(center, 1.0f));

            centerLS.x = std::floor(centerLS.x / texel) * texel;
            centerLS.y = std::floor(centerLS.y / texel) * texel;

            const float nearDist = -centerLS.z - radius - s.casterExtension;
            const float farDist  = -centerLS.z + radius;

            const glm::mat4 proj = glm::orthoRH_ZO(centerLS.x - halfExtent, centerLS.x + halfExtent,
                                                   centerLS.y - halfExtent, centerLS.y + halfExtent,
                                                   nearDist, farDist);
            out.viewProj[i] = proj * lightRot;
            out.texelWorld[i] = texel;
            out.depthRange[i] = farDist - nearDist;
        }

        for (uint32_t i = out.count; i < MAX_SHADOW_CASCADES; ++i) {
            out.viewProj[i] = out.viewProj[out.count - 1];
            out.texelWorld[i] = out.texelWorld[out.count - 1];
            out.depthRange[i] = out.depthRange[out.count - 1];
        }
        return out;
    }

    // point / area lights

    struct LocalShadowData {
        std::array<glm::mat4, SHADOW_MAP_LAYERS> viewProj{};
        uint32_t layerCount = 0;
        float nearPlane = 0.1f;
        float farPlane = 1.0f;
        float tanHalfFov = 1.0f;
    };

    constexpr float POINT_SHADOW_TAN_HALF_FOV = 1.04f;

    inline LocalShadowData computePointLightShadow(const glm::vec3& position, float range) {
        LocalShadowData out{};
        range = std::max(range, 0.1f);
        out.nearPlane = std::max(range * 0.01f, 0.05f);
        out.farPlane = std::max(range, out.nearPlane + 0.1f);
        out.tanHalfFov = POINT_SHADOW_TAN_HALF_FOV;
        out.layerCount = SHADOW_CUBE_FACES;

        const float fovY = 2.0f * std::atan(out.tanHalfFov);
        const glm::mat4 proj = glm::perspectiveRH_ZO(fovY, 1.0f, out.nearPlane, out.farPlane);

        const std::array<glm::vec3, 6> dirs = {
            glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0),
            glm::vec3(0, 1, 0), glm::vec3(0, -1, 0),
            glm::vec3(0, 0, 1), glm::vec3(0, 0, -1)
        };
        const std::array<glm::vec3, 6> ups = {
            glm::vec3(0, 1, 0), glm::vec3(0, 1, 0),
            glm::vec3(0, 0, 1), glm::vec3(0, 0, 1),
            glm::vec3(0, 1, 0), glm::vec3(0, 1, 0)
        };
        for (uint32_t f = 0; f < SHADOW_CUBE_FACES; ++f) {
            out.viewProj[f] = proj * glm::lookAtRH(position, position + dirs[f], ups[f]);
        }
        return out;
    }

    // single frustum
    inline LocalShadowData computeSpotLightShadow(const glm::vec3& position, const glm::vec3& aimDirection,
                                                  float range, float fovDegrees) {
        LocalShadowData out{};
        const glm::vec3 aim = glm::length(aimDirection) > 0.0001f ? glm::normalize(aimDirection)
                                                                  : glm::vec3(0.0f, -1.0f, 0.0f);
        const glm::vec3 upHint = (std::abs(aim.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, 1.0f)
                                                            : glm::vec3(0.0f, 1.0f, 0.0f);
        range = std::max(range, 0.1f);
        out.nearPlane = std::max(range * 0.01f, 0.05f);
        out.farPlane = std::max(range, out.nearPlane + 0.1f);
        const float fovY = glm::radians(std::clamp(fovDegrees, 1.0f, 170.0f));
        out.tanHalfFov = std::tan(fovY * 0.5f);
        out.layerCount = 1;

        out.viewProj[0] = glm::perspectiveRH_ZO(fovY, 1.0f, out.nearPlane, out.farPlane)
                        * glm::lookAtRH(position, position + aim, upHint);
        return out;
    }

    // caster culling

    struct FrustumPlanes {
        std::array<glm::vec4, 6> planes{};
    };

    inline FrustumPlanes extractFrustumPlanes(const glm::mat4& m) {
        auto row = [&m](int i) { return glm::vec4(m[0][i], m[1][i], m[2][i], m[3][i]); };
        const glm::vec4 r0 = row(0), r1 = row(1), r2 = row(2), r3 = row(3);

        FrustumPlanes f;
        f.planes[0] = r3 + r0;
        f.planes[1] = r3 - r0;
        f.planes[2] = r3 + r1;
        f.planes[3] = r3 - r1;
        f.planes[4] = r2;
        f.planes[5] = r3 - r2;

        for (auto& p : f.planes) {
            const float len = glm::length(glm::vec3(p));
            if (len > 0.0f) p /= len;
        }
        return f;
    }

    inline bool sphereIntersectsFrustum(const FrustumPlanes& f, const glm::vec3& center, float radius) {
        for (const auto& p : f.planes) {
            if (glm::dot(glm::vec3(p), center) + p.w < -radius) return false;
        }
        return true;
    }

}

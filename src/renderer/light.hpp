#pragma once

#include <glm/glm.hpp>
#include <cstdint>

namespace slate {

    enum class LightType : int32_t {
        Sun   = 0,
        Point = 1,
        Area  = 2
    };

    struct SceneLight {
        LightType type = LightType::Sun;

        // sun
        glm::vec3 sunDirection = glm::normalize(glm::vec3(1.0f, 2.0f, 1.5f));
        float sunOrthoHalfExtent = 24.0f;
        float sunShadowDepthMultiplier = 6.0f;

        // point / area
        glm::vec3 position{0.0f, 8.0f, 0.0f};
        glm::vec3 aimDirection = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));
        float range = 25.0f;
        float shadowFovDegrees = 90.0f;

        // shared
        glm::vec3 color{1.0f, 0.95f, 0.9f};
        float intensity = 300.0f;
        float shadowSoftness = 1.0f;
    };

    constexpr uint32_t MAX_DYNAMIC_LIGHTS = 16;

    struct DynamicPointLight {
        glm::vec3 position{0.0f};
        float range = 10.0f;
        glm::vec3 color{1.0f, 1.0f, 1.0f};
        float intensity = 1.0f;
    };

    struct alignas(16) DynamicPointLightGPU {
        glm::vec3 position{0.0f};
        float range = 10.0f;
        glm::vec3 color{1.0f, 1.0f, 1.0f};
        float intensity = 1.0f;
    };
    static_assert(sizeof(DynamicPointLightGPU) == 32, "DynamicPointLightGPU must match pbr.frag std430 layout");

    struct alignas(16) DynamicLightBufferGPU {
        int32_t count = 0;
        int32_t _pad[3]{};
        DynamicPointLightGPU lights[MAX_DYNAMIC_LIGHTS]{};
    };
    static_assert(sizeof(DynamicLightBufferGPU) == 16 + 32 * MAX_DYNAMIC_LIGHTS, "DynamicLightBufferGPU size mismatch");

}
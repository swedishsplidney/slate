#pragma once

#include <glm/glm.hpp>
#include <cstdint>

namespace slate {

    enum class LightType : int32_t {
        Sun   = 0,
        Point = 1,
        Area  = 2
    };

    // one dynamic light
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


        glm::vec3 color{1.0f, 0.95f, 0.9f};
        float intensity = 3.0f;
        float shadowSoftness = 1.0f;
    };

}
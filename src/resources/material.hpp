#pragma once

#include <glm/glm.hpp>
#include <string>

namespace slate {

    struct alignas(16) MaterialGPU {
        glm::vec4 albedoFactor{1.0f};
        float roughnessFactor{0.5f};
        float metallicFactor{0.0f};
        float transmissionFactor{0.0f};
        float ior{1.45f};
        float padding[4]{0.0f, 0.0f, 0.0f, 0.0f};
    };

    struct Material {
        std::string name;
        uint32_t materialId = 0;
        MaterialGPU gpuData;
    };

}
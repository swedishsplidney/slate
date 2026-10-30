#pragma once

#include <glm/vec4.hpp>

namespace slate::theme {

    // surfaces
    inline const glm::vec4 panel     {0.004f, 0.0045f, 0.0055f, 1.0f};
    inline const glm::vec4 header    {0.016f, 0.018f,  0.023f,  1.0f};
    inline const glm::vec4 section   {0.010f, 0.011f,  0.015f,  1.0f};
    inline const glm::vec4 menuBar   {0.010f, 0.011f,  0.013f,  1.0f};
    inline const glm::vec4 popup     {0.016f, 0.018f,  0.023f,  0.95f};

    // fields
    inline const glm::vec4 field     {0.032f, 0.036f,  0.046f,  1.0f};
    inline const glm::vec4 fieldFocus{0.060f, 0.070f,  0.090f,  1.0f};

    // interaction
    inline const glm::vec4 selection {0.200f, 0.250f,  0.400f,  1.0f};
    inline const glm::vec4 accent    {0.250f, 0.350f,  0.550f,  1.0f};

    // text
    inline const glm::vec4 text      {0.850f, 0.850f,  0.900f,  1.0f};
    inline const glm::vec4 textDim   {0.550f, 0.550f,  0.600f,  1.0f};
    inline const glm::vec4 menuText  {0.250f, 0.250f,  0.280f,  1.0f};

    // metrics
    inline constexpr float headerH     = 28.0f;
    inline constexpr float rowH        = 24.0f;
    inline constexpr float fieldH      = 22.0f;
    inline constexpr float gap         = 8.0f;
    inline constexpr float radius      = 3.0f;
    inline constexpr float radiusPopup = 4.0f;

}

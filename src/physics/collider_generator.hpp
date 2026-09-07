#pragma once

#include <vector>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include "renderer/vulkan/vulkan_renderer.hpp"

namespace slate {

    class ColliderGenerator {
    public:
        // generates actually good colliders that hopefully dont suck :) (i tried my best ok)
        static JPH::ShapeSettings::ShapeResult createOptimalShape(const std::vector<Vertex>& vertices);
    };

}
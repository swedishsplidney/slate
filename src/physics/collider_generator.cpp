#include "collider_generator.hpp"
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

namespace slate {

    JPH::ShapeSettings::ShapeResult ColliderGenerator::createOptimalShape(const std::vector<Vertex>& vertices) {
        if (vertices.empty()) {
            JPH::BoxShapeSettings boxSettings(JPH::Vec3(0.5f, 0.5f, 0.5f));
            return boxSettings.Create();
        }

        JPH::Array<JPH::Vec3> joltVertices;
        joltVertices.reserve(vertices.size());

        for (const auto& v : vertices) {
            joltVertices.push_back(JPH::Vec3(v.pos.x, v.pos.y, v.pos.z));
        }

        JPH::ConvexHullShapeSettings hullSettings(joltVertices);
        return hullSettings.Create();
    }

}
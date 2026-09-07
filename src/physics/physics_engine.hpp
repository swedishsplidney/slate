#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>

namespace slate {

    namespace Layers {
        static constexpr JPH::ObjectLayer NON_MOVING = 0;
        static constexpr JPH::ObjectLayer MOVING = 1;
        static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
    }

    class PhysicsEngine {
    public:
        PhysicsEngine();
        ~PhysicsEngine();

        void init();
        void update(float deltaTime);
        void shutdown();

        JPH::PhysicsSystem& getPhysicsSystem() { return m_physicsSystem; }

    private:
        class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
        public:
            BPLayerInterfaceImpl();
            uint32_t GetNumBroadPhaseLayers() const override;
            JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override;
            
            const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override;

        private:
            JPH::BroadPhaseLayer m_objectToBroadPhase[Layers::NUM_LAYERS];
        };

        class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
        public:
            bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override;
        };

        class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
        public:
            bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::ObjectLayer inLayer2) const override;
        };

        std::unique_ptr<JPH::TempAllocatorImpl> m_tempAllocator;
        std::unique_ptr<JPH::JobSystemThreadPool> m_jobSystem;
        JPH::PhysicsSystem m_physicsSystem;

        BPLayerInterfaceImpl m_broadPhaseLayerInterface;
        ObjectVsBroadPhaseLayerFilterImpl m_objectVsBroadPhaseLayerFilter;
        ObjectLayerPairFilterImpl m_objectLayerPairFilter;
    };

}
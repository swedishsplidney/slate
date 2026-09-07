#include "physics_engine.hpp"
#include <iostream>
#include <cstdarg>

static void JoltTraceCallback(const char* inFMT, ...) {
    va_list list;
    va_start(list, inFMT);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), inFMT, list);
    va_end(list);
    std::cout << "[jolt trace] " << buffer << std::endl;
}

#ifdef JPH_ENABLE_ASSERTS
static bool JoltAssertFailedCallback(const char* inExpression, const char* inMessage, const char* inFile, JPH::uint inLine) {
    std::cerr << "[jolt assert failure] " << inFile << ":" << inLine << " (" << inExpression << ") " << (inMessage ? inMessage : "") << std::endl;
    return true; 
}
#endif

namespace slate {

    PhysicsEngine::BPLayerInterfaceImpl::BPLayerInterfaceImpl() {
        m_objectToBroadPhase[Layers::NON_MOVING] = JPH::BroadPhaseLayer(0);
        m_objectToBroadPhase[Layers::MOVING] = JPH::BroadPhaseLayer(1);
    }

    uint32_t PhysicsEngine::BPLayerInterfaceImpl::GetNumBroadPhaseLayers() const {
        return 2;
    }

    JPH::BroadPhaseLayer PhysicsEngine::BPLayerInterfaceImpl::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const {
        JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
        return m_objectToBroadPhase[inLayer];
    }

    const char* PhysicsEngine::BPLayerInterfaceImpl::GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const {
        switch (inLayer.GetValue()) {
            case 0: return "NON_MOVING";
            case 1: return "MOVING";
            default: return "UNKNOWN";
        }
    }

    bool PhysicsEngine::ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const {
        switch (inLayer1) {
            case Layers::NON_MOVING:
                return inLayer2 == JPH::BroadPhaseLayer(1);
            case Layers::MOVING:
                return true;
            default:
                JPH_ASSERT(false);
                return false;
        }
    }

    bool PhysicsEngine::ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::ObjectLayer inLayer2) const {
        switch (inLayer1) {
            case Layers::NON_MOVING:
                return inLayer2 == Layers::MOVING;
            case Layers::MOVING:
                return true;
            default:
                JPH_ASSERT(false);
                return false;
        }
    }

    PhysicsEngine::PhysicsEngine() = default;

    PhysicsEngine::~PhysicsEngine() {
        shutdown();
    }

    void PhysicsEngine::init() {
        JPH::RegisterDefaultAllocator();

        JPH::Trace = JoltTraceCallback;
#ifdef JPH_ENABLE_ASSERTS
        JPH::AssertFailed = JoltAssertFailedCallback;
#endif

        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();

        m_tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
        m_jobSystem = std::make_unique<JPH::JobSystemThreadPool>(
            JPH::cMaxPhysicsJobs, 
            JPH::cMaxPhysicsBarriers, 
            std::thread::hardware_concurrency() - 1
        );

        const uint32_t cMaxBodies = 1024;
        const uint32_t cNumBodyMutexes = 0;
        const uint32_t cMaxBodyPairs = 1024;
        const uint32_t cMaxContactConstraints = 1024;

        m_physicsSystem.Init(
            cMaxBodies, 
            cNumBodyMutexes, 
            cMaxBodyPairs, 
            cMaxContactConstraints, 
            m_broadPhaseLayerInterface, 
            m_objectVsBroadPhaseLayerFilter, 
            m_objectLayerPairFilter
        );

        std::cout << "[physics engine] jolt physics initialized successfully!\n";
    }

    void PhysicsEngine::update(float deltaTime) {
        const int cCollisionSteps = 1;
        m_physicsSystem.Update(deltaTime, cCollisionSteps, m_tempAllocator.get(), m_jobSystem.get());
    }

    void PhysicsEngine::shutdown() {
        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;

        std::cout << "[physics engine] jolt physics shut down\n";
    }

}
#pragma once
#include <cstdint>
#include <DirectXMath.h>
#include <deps/glm/glm.hpp>

namespace triggerbot {
    // Settings
    extern bool enabled;
    extern float delayMs;        // Délai avant tir (ms)
    extern float maxDistance;  // Distance max pour trigger
    extern float headOnly;     // 0 = tout le corps, 1 = head only
    
    // Functions
    void Run(uint64_t localPlayer, uint64_t entityList);
    bool IsEnemyInCrosshair(uint64_t localPlayer, uint64_t targetBipedPose, 
                            const DirectX::XMFLOAT3X4& targetTransform,
                            const glm::vec3& cameraPos, const glm::vec3& cameraForward);
    void TriggerFire();
}

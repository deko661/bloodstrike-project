#include "triggerbot.hpp"
#include "offsets.hpp"
#include <deps/math/math.hpp>
#include <deps/glm/glm.hpp>
#include <deps/memory/memory.h>
#include <Windows.h>
#include <thread>
#include <chrono>

extern std::unique_ptr<c_memory> memory;

namespace triggerbot {
    bool enabled = false;
    float delayMs = 50.0f;
    float maxDistance = 100.0f;
    float headOnly = 0.0f;
    
    static uint64_t lastTarget = 0;
    static auto lastFireTime = std::chrono::steady_clock::now();
    
    bool ReadBonePos(uint64_t bipedPose, const DirectX::XMFLOAT3X4& transform, int boneIndex, glm::vec3& outPos)
    {
        uint64_t boneStart = memory->read<uint64_t>(bipedPose + boneIndex * 0x8);
        if (!boneStart) return false;
        
        DirectX::XMFLOAT3X4 boneMatrix = memory->read<DirectX::XMFLOAT3X4>(boneStart + offsets::bone::boneMatrix);
        MessiahMatrixAdd(boneMatrix, transform, outPos);
        return true;
    }
    
    void TriggerFire()
    {
        INPUT inputDown = {};
        inputDown.type = INPUT_MOUSE;
        inputDown.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &inputDown, sizeof(INPUT));
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        INPUT inputUp = {};
        inputUp.type = INPUT_MOUSE;
        inputUp.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &inputUp, sizeof(INPUT));
    }
    
    bool IsPointInCrosshair(const glm::vec3& cameraPos, const glm::vec3& cameraForward, 
                            const glm::vec3& targetPos, float threshold)
    {
        glm::vec3 toTarget = targetPos - cameraPos;
        float dist = glm::length(toTarget);
        
        if (dist > maxDistance) return false;
        
        glm::vec3 dirToTarget = glm::normalize(toTarget);
        
        float dot = glm::dot(cameraForward, dirToTarget);
        
        return dot > threshold;
    }
    
    bool IsEnemyInCrosshair(uint64_t localPlayer, uint64_t targetBipedPose, 
                            const DirectX::XMFLOAT3X4& targetTransform,
                            const glm::vec3& cameraPos, const glm::vec3& cameraForward)
    {
        glm::vec3 headPos;
        if (ReadBonePos(targetBipedPose, targetTransform, offsets::bone::head, headPos)) {
            if (IsPointInCrosshair(cameraPos, cameraForward, headPos, 0.995f)) {
                return true;
            }
        }
        
        if (headOnly < 0.5f) {
            glm::vec3 chestPos;
            if (ReadBonePos(targetBipedPose, targetTransform, offsets::bone::spine3, chestPos)) {
                if (IsPointInCrosshair(cameraPos, cameraForward, chestPos, 0.985f)) {
                    return true;
                }
            }
            
            glm::vec3 pelvisPos;
            if (ReadBonePos(targetBipedPose, targetTransform, offsets::bone::pelvis, pelvisPos)) {
                if (IsPointInCrosshair(cameraPos, cameraForward, pelvisPos, 0.98f)) {
                    return true;
                }
            }
        }
        
        return false;
    }
    
    void Run(uint64_t localPlayer, uint64_t entityList)
    {
        if (!enabled || !localPlayer || !entityList) return;
        
        uint64_t camera = memory->read<uint64_t>(localPlayer + offsets::player::camera);
        if (!camera) return;
        
        glm::vec3 camPos = memory->read<glm::vec3>(camera + 0x70);
        glm::vec3 camForward = memory->read<glm::vec3>(camera + 0x50);
        
        camForward = glm::normalize(camForward);
        
        uint64_t currentActor = memory->read<uint64_t>(entityList + offsets::entitylist::head);
        
        while (currentActor) {
            if (currentActor == localPlayer) {
                currentActor = memory->read<uint64_t>(currentActor);
                continue;
            }
            
            uint64_t IEntity = memory->read<uint64_t>(currentActor + offsets::entity::IEntity);
            if (!IEntity) { currentActor = memory->read<uint64_t>(currentActor); continue; }
            
            uint64_t actorInstance = memory->read<uint64_t>(IEntity + offsets::entity::actorInstance);
            if (!actorInstance) { currentActor = memory->read<uint64_t>(currentActor); continue; }
            
            uint64_t actorProps = memory->read<uint64_t>(actorInstance + offsets::entity::actorProps);
            if (!actorProps) { currentActor = memory->read<uint64_t>(currentActor); continue; }
            
            bool isAlive = memory->read<bool>(actorProps + 0x2dc);
            if (!isAlive) { currentActor = memory->read<uint64_t>(currentActor); continue; }
            
            int myTeam = memory->read<int>(localPlayer + offsets::player::teamID);
            int theirTeam = memory->read<int>(currentActor + offsets::player::teamID);
            if (myTeam == theirTeam) { currentActor = memory->read<uint64_t>(currentActor); continue; }
            
            DirectX::XMFLOAT3X4 transform = memory->read<DirectX::XMFLOAT3X4>(IEntity + offsets::entity::transform);
            
            uint64_t BipedPose = memory->read<uint64_t>(actorProps + offsets::entity::BipedPose);
            if (!BipedPose) { currentActor = memory->read<uint64_t>(currentActor); continue; }
            BipedPose += offsets::entity::BipedPoseBase;
            
            if (IsEnemyInCrosshair(localPlayer, BipedPose, transform, camPos, camForward)) {
                auto now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFireTime).count();
                
                if (elapsed > static_cast<int>(delayMs) + 100) {
                    if (delayMs > 0) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(delayMs)));
                    }
                    
                    if (IsEnemyInCrosshair(localPlayer, BipedPose, transform, camPos, camForward)) {
                        TriggerFire();
                        lastFireTime = std::chrono::steady_clock::now();
                        break;
                    }
                }
            }
            
            currentActor = memory->read<uint64_t>(currentActor);
        }
    }
}

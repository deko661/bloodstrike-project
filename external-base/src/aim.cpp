#include "aim.hpp"
#include "settings.hpp"
#include "esp.hpp"
#include "triggerbot.hpp"
#include <deps/math/math.hpp>
#include "offsets.hpp"
#include <deps/glm/glm.hpp>
#include <deps/memory/memory.h>
#include <Windows.h>
#include <cmath>

extern std::unique_ptr<c_memory> memory;

namespace bloodstrike
{
    namespace renderer
    {
        extern uint64_t camera;
        extern uint64_t localActor;
    }
}

namespace aim
{
    glm::vec2 currentTarget(0.0f, 0.0f);
    bool hasTarget = false;

    void FindTarget()
    {
        hasTarget = false;
        currentTarget = glm::vec2(0.0f, 0.0f);

        if (!(GetAsyncKeyState(sdk::aimbotKey) & 0x8000)) return;

        glm::vec3 local_pos = get_local_position();

        uint64_t base = memory->get_module_address();
        uint64_t ClientEngine = memory->read<uint64_t>(base + offsets::module::ClientEngineSteam);
        if (!ClientEngine) return;
        uint64_t IGameplay = memory->read<uint64_t>(ClientEngine + offsets::player::IGameplay);
        if (!IGameplay) return;
        uint64_t ClientPlayer = memory->read<uint64_t>(IGameplay + offsets::player::ClientPlayer);
        if (!ClientPlayer) return;
        bloodstrike::renderer::camera = memory->read<uint64_t>(ClientPlayer + offsets::player::camera);
        bloodstrike::renderer::localActor = memory->read<uint64_t>(ClientPlayer + offsets::player::localActor);
        if (!bloodstrike::renderer::camera || !bloodstrike::renderer::localActor) return;

        uint64_t entityListStart = memory->read<uint64_t>(base + offsets::module::EntityListSteam);
        if (!entityListStart) return;
        uint64_t head = memory->read<uint64_t>(entityListStart + offsets::entitylist::head);
        if (!head) return;
        uint64_t currentActor = memory->read<uint64_t>(head);

        int localTeam = 0;
        if (sdk::aimbotTeamCheck && bloodstrike::renderer::localActor)
        {
            localTeam = memory->read<int>(bloodstrike::renderer::localActor + offsets::player::teamID);
        }

        float screenCenterX = GetSystemMetrics(SM_CXSCREEN) / 2.0f;
        float screenCenterY = GetSystemMetrics(SM_CYSCREEN) / 2.0f;
        float closestDistance = sdk::aimbotFov;

        if (currentActor)
        {
            do
            {
                uint64_t actorInstance = memory->read<uint64_t>(currentActor + offsets::entity::actorInstance);
                if (!actorInstance) { currentActor = memory->read<uint64_t>(currentActor); continue; }
                uint64_t actorProps = memory->read<uint64_t>(actorInstance + offsets::entity::actorProps);
                if (!actorProps) { currentActor = memory->read<uint64_t>(currentActor); continue; }
                uint64_t actorComponent = memory->read<uint64_t>(actorProps + offsets::entity::actorComponent);
                if (!actorComponent) { currentActor = memory->read<uint64_t>(currentActor); continue; }
                uint64_t IEntity = memory->read<uint64_t>(actorComponent + offsets::entity::IEntity);
                if (!IEntity) { currentActor = memory->read<uint64_t>(currentActor); continue; }
                uint64_t entityMask = memory->read<uint64_t>(IEntity + offsets::entity::entityMask);
                if (entityMask != 2) { currentActor = memory->read<uint64_t>(currentActor); continue; }
                int theirTeam = memory->read<int>(IEntity + offsets::player::teamID);
                if (sdk::aimbotTeamCheck && localTeam != 0 && theirTeam == localTeam) { currentActor = memory->read<uint64_t>(currentActor); continue; }
                if (IEntity == bloodstrike::renderer::localActor) { currentActor = memory->read<uint64_t>(currentActor); continue; }
                uint64_t IArea = memory->read<uint64_t>(IEntity + offsets::entity::IArea);
                if (IArea == 0x0) { currentActor = memory->read<uint64_t>(currentActor); continue; }
                uint64_t pose = memory->read<uint64_t>(actorInstance + offsets::entity::pose);
                if (!pose) { currentActor = memory->read<uint64_t>(currentActor); continue; }
                uint64_t BipedPose = memory->read<uint64_t>(pose + offsets::entity::BipedPose);
                if (!BipedPose) { currentActor = memory->read<uint64_t>(currentActor); continue; }
                BipedPose += offsets::entity::BipedPoseBase;

                int boneOffset = offsets::bone::neck;
                switch(sdk::aimbotTargetBone)
                {
                case 0: boneOffset = offsets::bone::head; break;
                case 1: case 9: boneOffset = offsets::bone::neck; break;
                case 2: case 10: boneOffset = offsets::bone::spine1; break;
                case 3: case 11: boneOffset = offsets::bone::spine2; break;
                case 12: boneOffset = offsets::bone::spine3; break;
                case 4: case 14: boneOffset = offsets::bone::pelvis; break;
                case 5: boneOffset = offsets::bone::wristL; break;
                case 6: boneOffset = offsets::bone::wristR; break;
                case 7: boneOffset = offsets::bone::footL; break;
                case 8: boneOffset = offsets::bone::footR; break;
                case 13: boneOffset = offsets::bone::sholL; break;
                default: boneOffset = offsets::bone::neck; break;
                }

                uint64_t targetBoneStart = memory->read<uint64_t>(BipedPose + (boneOffset * 0x8));
                if (!targetBoneStart) { currentActor = memory->read<uint64_t>(currentActor); continue; }
                DirectX::XMFLOAT3X4 dxTrans = memory->read<DirectX::XMFLOAT3X4>(IEntity + offsets::entity::transform);
                glm::vec3 targetPos;
                MessiahMatrixAdd(memory->read<DirectX::XMFLOAT3X4>(targetBoneStart + offsets::bone::boneMatrix), dxTrans, targetPos);
                glm::vec2 target2D;
                if (w2s(bloodstrike::renderer::camera, targetPos, target2D, false))
                {
                    float dx = targetPos.x - local_pos.x;
                    float dy = targetPos.y - local_pos.y;
                    float dz = targetPos.z - local_pos.z;
                    float dist = sqrtf(dx * dx + dy * dy + dz * dz) / 5.0f;

                    if (dist <= sdk::espMaxDistance)
                    {
                        float distToCrosshair = std::sqrt(std::pow(target2D.x - screenCenterX, 2) + std::pow(target2D.y - screenCenterY, 2));
                                        float score = distToCrosshair + (dist * 0.1f);
                        if (score < closestDistance)
                        {
                            closestDistance = score;
                            currentTarget = target2D;
                            hasTarget = true;
                        }
                    }
                }
                currentActor = memory->read<uint64_t>(currentActor);
            } while (currentActor != head && currentActor != 0);
        }
    }

    void AimAt(const glm::vec2& target)
    {
        float screenCenterX = GetSystemMetrics(SM_CXSCREEN) / 2.0f;
        float screenCenterY = GetSystemMetrics(SM_CYSCREEN) / 2.0f;

        float deltaX = target.x - screenCenterX;
        float deltaY = target.y - screenCenterY;
        if (sdk::aimbotSmooth > 0.0f)
        {
            deltaX /= sdk::aimbotSmooth;
            deltaY /= sdk::aimbotSmooth;
        }
        mouse_event(MOUSEEVENTF_MOVE, (DWORD)deltaX, (DWORD)deltaY, 0, 0);
    }

    void FindTargetViaSingleton()
    {
        hasTarget = false;
        currentTarget = glm::vec2(0.0f, 0.0f);

        if (!(GetAsyncKeyState(sdk::aimbotKey) & 0x8000)) return;

        glm::vec3 local_pos = get_local_position();
        uint64_t base = memory->get_module_address();

        uint64_t ientityMgr = memory->read<uint64_t>(base + offsets::singleton::MessiahIEntity);
        if (!ientityMgr) return;

        uint64_t entityArray = memory->read<uint64_t>(ientityMgr + 0x0);
        uint64_t entityCount = memory->read<uint64_t>(ientityMgr + 0x8);

        if (!entityArray || entityCount == 0) return;

        uint64_t camera = 0;
        uint64_t localActor = 0;

        uint64_t icamera = memory->read<uint64_t>(base + offsets::singleton::MessiahICamera);
        if (icamera) {
            camera = memory->read<uint64_t>(icamera + 0x0);
        }

        if (!camera) {
            uint64_t ClientEngine = memory->read<uint64_t>(base + offsets::module::ClientEngineSteam);
            if (!ClientEngine) return;
            uint64_t IGameplay = memory->read<uint64_t>(ClientEngine + offsets::player::IGameplay);
            if (!IGameplay) return;
            uint64_t ClientPlayer = memory->read<uint64_t>(IGameplay + offsets::player::ClientPlayer);
            if (!ClientPlayer) return;
            camera = memory->read<uint64_t>(ClientPlayer + offsets::player::camera);
            localActor = memory->read<uint64_t>(ClientPlayer + offsets::player::localActor);
        }

        if (!camera) return;
        bloodstrike::renderer::camera = camera;
        bloodstrike::renderer::localActor = localActor;

        int localTeam = 0;
        if (sdk::aimbotTeamCheck && localActor) {
            localTeam = memory->read<int>(localActor + offsets::player::teamID);
        }

        float screenCenterX = GetSystemMetrics(SM_CXSCREEN) / 2.0f;
        float screenCenterY = GetSystemMetrics(SM_CYSCREEN) / 2.0f;
        float closestDistance = sdk::aimbotFov;

        for (uint64_t i = 0; i < entityCount && i < 100; i++) {
            uint64_t IEntity = memory->read<uint64_t>(entityArray + i * 0x8);
            if (!IEntity) continue;

            uint64_t entityMask = memory->read<uint64_t>(IEntity + offsets::entity::entityMask);
            if (entityMask != 2) continue;

            if (IEntity == localActor) continue;

            int theirTeam = memory->read<int>(IEntity + offsets::player::teamID);
            if (sdk::aimbotTeamCheck && localTeam != 0 && theirTeam == localTeam) continue;

            uint64_t IArea = memory->read<uint64_t>(IEntity + offsets::entity::IArea);
            if (IArea == 0x0) continue;
            int boneOffset = offsets::bone::neck;
            switch(sdk::aimbotTargetBone) {
            case 0: boneOffset = offsets::bone::head; break;
            case 1: case 9: boneOffset = offsets::bone::neck; break;
            case 2: case 10: boneOffset = offsets::bone::spine1; break;
            case 3: case 11: boneOffset = offsets::bone::spine2; break;
            case 12: boneOffset = offsets::bone::spine3; break;
            case 4: case 14: boneOffset = offsets::bone::pelvis; break;
            case 5: boneOffset = offsets::bone::wristL; break;
            case 6: boneOffset = offsets::bone::wristR; break;
            case 7: boneOffset = offsets::bone::footL; break;
            case 8: boneOffset = offsets::bone::footR; break;
            case 13: boneOffset = offsets::bone::sholL; break;
            default: boneOffset = offsets::bone::neck; break;
            }

            uint64_t BipedPose = 0;

            uint64_t skelComp = memory->read<uint64_t>(base + offsets::singleton::MessiahSkeletonComponent);
            if (skelComp) {
                BipedPose = memory->read<uint64_t>(IEntity + offsets::entity::pose + offsets::entity::BipedPose);
            }

            if (!BipedPose) {
                uint64_t actorComponent = memory->read<uint64_t>(IEntity + offsets::entity::actorComponent);
                if (actorComponent) {
                    uint64_t actorProps = memory->read<uint64_t>(actorComponent + offsets::entity::actorProps);
                    if (actorProps) {
                        uint64_t pose = memory->read<uint64_t>(actorProps + offsets::entity::pose);
                        if (pose) {
                            BipedPose = memory->read<uint64_t>(pose + offsets::entity::BipedPose);
                        }
                    }
                }
            }

            if (!BipedPose) continue;
            BipedPose += offsets::entity::BipedPoseBase;

            uint64_t targetBoneStart = memory->read<uint64_t>(BipedPose + (boneOffset * 0x8));
            if (!targetBoneStart) continue;

            DirectX::XMFLOAT3X4 dxTrans = memory->read<DirectX::XMFLOAT3X4>(IEntity + offsets::entity::transform);
            glm::vec3 targetPos;
            MessiahMatrixAdd(memory->read<DirectX::XMFLOAT3X4>(targetBoneStart + offsets::bone::boneMatrix), dxTrans, targetPos);

            glm::vec2 target2D;
            if (w2s(camera, targetPos, target2D, false)) {
                float dx = targetPos.x - local_pos.x;
                float dy = targetPos.y - local_pos.y;
                float dz = targetPos.z - local_pos.z;
                float dist = sqrtf(dx * dx + dy * dy + dz * dz) / 5.0f;

                if (dist <= sdk::espMaxDistance) {
                    float distToCrosshair = std::sqrt(std::pow(target2D.x - screenCenterX, 2) + std::pow(target2D.y - screenCenterY, 2));
                    // Combine distance to crosshair AND 3D distance (closer players get priority)
                    float score = distToCrosshair + (dist * 0.1f); // 0.1f weight for 3D distance
                    if (score < closestDistance) {
                        closestDistance = score;
                        currentTarget = target2D;
                        hasTarget = true;
                    }
                }
            }
        }
    }

    void Run()
    {
        if (!sdk::aimbotEnabled) return;
        FindTarget();
        if (hasTarget)
        {
            AimAt(currentTarget);
        }
        
        if (triggerbot::enabled && bloodstrike::renderer::camera && bloodstrike::renderer::localActor)
        {
            uint64_t base = memory->get_module_address();
            uint64_t entityListStart = memory->read<uint64_t>(base + offsets::module::EntityListSteam);
            triggerbot::Run(bloodstrike::renderer::localActor, entityListStart);
        }
    }
}

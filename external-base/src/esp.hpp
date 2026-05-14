#pragma once
#include <cstdint>
#include <DirectXMath.h>
#include <deps/glm/glm.hpp>
#include <deps/imgui/imgui.h>
#include <deps/imgui/imgui_internal.h> // for ImColor, ImVec2, ImDrawList

namespace esp
{
    void Render();
    void DrawFovCircle();

    struct BoneScreen
    {
        // 2D screen positions
        glm::vec2 head, neck, spine1, spine2, spine3, pelvis;
        glm::vec2 sholL, elbowL, wristL;
        glm::vec2 sholR, elbowR, wristR;
        glm::vec2 buttL, kneeL, footL;
        glm::vec2 buttR, kneeR, footR;
        
        // 3D world positions (for real 3D box)
        glm::vec3 head3D, neck3D, pelvis3D;
        glm::vec3 footL3D, footR3D;
        bool has3D = false;
        
        bool hasHead, hasNeck, hasSpine1, hasSpine2, hasSpine3, hasPelvis;
        bool hasSholL, hasElbowL, hasWristL;
        bool hasSholR, hasElbowR, hasWristR;
        bool hasButtL, hasKneeL, hasFootL;
        bool hasButtR, hasKneeR, hasFootR;
    };

    bool GetBonesForPlayer(uint64_t bipedPose, const DirectX::XMFLOAT3X4& transform, BoneScreen& bones);
    void DrawSkeleton(const BoneScreen& bones, ImColor color);
    void DrawChapeauChinois3D(const BoneScreen& bones, ImColor color);
    void DrawChapeauChinoisTrue3D(glm::vec3 neckPos, glm::vec3 headPos, uint64_t camera, ImColor color);
    void DrawBox(const BoneScreen& bones, ImColor color, float& outX, float& outY, float& outW, float& outH);
    void DrawTracers(float targetX, float targetY, bool useBox, float boxX, float boxY, float boxW, float boxH, ImColor color);
    void DrawDistance(float distance, float targetX, float targetY, bool useBox, float boxX, float boxY, float boxW, float boxH);

    // New: Render via MessiahIEntity singleton (no EntityList scan needed)
    void RenderPlayersViaSingleton();

    // Loot ESP
    void RenderLoot();

    // Bot detection helper
    bool IsBot(uint64_t entity, uint64_t actorProps);

    // Debug: Test object list discovery
    void TestObjectListDiscovery();

    // Debug: Scan all entity types to discover loot/objects
    void DebugEntityScan();
}

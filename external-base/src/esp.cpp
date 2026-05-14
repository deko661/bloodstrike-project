#include "esp.hpp"
#include "settings.hpp"
#include "vector3.hpp"
#include <deps/math/math.hpp>
#include "offsets.hpp"
#include <deps/imgui/imgui.h>
#include <deps/memory/memory.h>
#include <algorithm>
#include <cmath>
#include <format>
#include <vector>

extern std::unique_ptr<c_memory> memory;

namespace bloodstrike
{
    namespace renderer
    {
        extern uint64_t camera;
        extern uint64_t localActor;
    }
}

extern glm::vec3 get_local_position();

namespace esp
{
    bool ReadBoneWorldPos(uint64_t bipedPose, const DirectX::XMFLOAT3X4& transform, int boneIndex, glm::vec3& outPos)
    {
        uint64_t boneStart = memory->read<uint64_t>(bipedPose + (boneIndex * 0x8));
        if (!boneStart) return false;

        DirectX::XMFLOAT3X4 boneMatrix = memory->read<DirectX::XMFLOAT3X4>(boneStart + offsets::bone::boneMatrix);
        MessiahMatrixAdd(boneMatrix, transform, outPos);
        return true;
    }

    bool ReadBonePos3D(uint64_t bipedPose, const DirectX::XMFLOAT3X4& transform, int boneIndex, glm::vec3& outPos, glm::vec2& outScreen)
    {
        if (!ReadBoneWorldPos(bipedPose, transform, boneIndex, outPos)) return false;
        return w2s(bloodstrike::renderer::camera, outPos, outScreen, false);
    }

    struct RadarEntry
    {
        uint64_t entity;
        glm::vec3 pos;
        bool isBot;
    };

    static std::vector<RadarEntry> g_radarEntries;

    void RadarBeginFrame()
    {
        g_radarEntries.clear();
    }

    void RadarAddEntity(uint64_t entity, const glm::vec3& pos, bool isBot)
    {
        if (!sdk::espRadar || !entity) return;

        for (const auto& entry : g_radarEntries) {
            if (entry.entity == entity) {
                return;
            }
        }

        g_radarEntries.push_back({ entity, pos, isBot });
    }

    bool GetBonesForPlayer(uint64_t bipedPose, const DirectX::XMFLOAT3X4& transform, BoneScreen& bones)
    {
        glm::vec3 pos;

        bones.hasHead = ReadBonePos3D(bipedPose, transform, offsets::bone::head, bones.head3D, bones.head);
        bones.hasNeck = ReadBonePos3D(bipedPose, transform, offsets::bone::neck, bones.neck3D, bones.neck);
        bones.hasSpine1 = ReadBonePos3D(bipedPose, transform, offsets::bone::spine1, pos, bones.spine1);
        bones.hasSpine2 = ReadBonePos3D(bipedPose, transform, offsets::bone::spine2, pos, bones.spine2);
        bones.hasSpine3 = ReadBonePos3D(bipedPose, transform, offsets::bone::spine3, pos, bones.spine3);
        bones.hasPelvis = ReadBonePos3D(bipedPose, transform, offsets::bone::pelvis, bones.pelvis3D, bones.pelvis);
        bones.hasSholL = ReadBonePos3D(bipedPose, transform, offsets::bone::sholL, pos, bones.sholL);
        bones.hasElbowL = ReadBonePos3D(bipedPose, transform, offsets::bone::elbowL, pos, bones.elbowL);
        bones.hasWristL = ReadBonePos3D(bipedPose, transform, offsets::bone::wristL, pos, bones.wristL);
        bones.hasSholR = ReadBonePos3D(bipedPose, transform, offsets::bone::sholR, pos, bones.sholR);
        bones.hasElbowR = ReadBonePos3D(bipedPose, transform, offsets::bone::elbowR, pos, bones.elbowR);
        bones.hasWristR = ReadBonePos3D(bipedPose, transform, offsets::bone::wristR, pos, bones.wristR);
        bones.hasButtL = ReadBonePos3D(bipedPose, transform, offsets::bone::buttCheekL, pos, bones.buttL);
        bones.hasKneeL = ReadBonePos3D(bipedPose, transform, offsets::bone::kneeL, pos, bones.kneeL);
        bones.hasFootL = ReadBonePos3D(bipedPose, transform, offsets::bone::footL, bones.footL3D, bones.footL);
        bones.hasButtR = ReadBonePos3D(bipedPose, transform, offsets::bone::buttCheekR, pos, bones.buttR);
        bones.hasKneeR = ReadBonePos3D(bipedPose, transform, offsets::bone::kneeR, pos, bones.kneeR);
        bones.hasFootR = ReadBonePos3D(bipedPose, transform, offsets::bone::footR, bones.footR3D, bones.footR);
        
        bones.has3D = bones.hasPelvis;

        return bones.hasHead || bones.hasNeck;
    }

    void DrawSkeleton(const BoneScreen& b, ImColor color)
    {
        auto drawLine = [&](const glm::vec2& a, const glm::vec2& b, bool validA, bool validB)
        {
            if (validA && validB)
                ImGui::GetBackgroundDrawList()->AddLine(ImVec2(a.x, a.y), ImVec2(b.x, b.y), color, 1.5f);
        };

        drawLine(b.neck, b.spine1, b.hasNeck, b.hasSpine1);
        drawLine(b.spine1, b.spine2, b.hasSpine1, b.hasSpine2);
        drawLine(b.spine2, b.spine3, b.hasSpine2, b.hasSpine3);
        drawLine(b.spine3, b.pelvis, b.hasSpine3, b.hasPelvis);

        drawLine(b.spine1, b.sholL, b.hasSpine1, b.hasSholL);
        drawLine(b.sholL, b.elbowL, b.hasSholL, b.hasElbowL);
        drawLine(b.elbowL, b.wristL, b.hasElbowL, b.hasWristL);

        drawLine(b.spine1, b.sholR, b.hasSpine1, b.hasSholR);
        drawLine(b.sholR, b.elbowR, b.hasSholR, b.hasElbowR);
        drawLine(b.elbowR, b.wristR, b.hasElbowR, b.hasWristR);

        drawLine(b.pelvis, b.buttL, b.hasPelvis, b.hasButtL);
        drawLine(b.buttL, b.kneeL, b.hasButtL, b.hasKneeL);
        drawLine(b.kneeL, b.footL, b.hasKneeL, b.hasFootL);

        drawLine(b.pelvis, b.buttR, b.hasPelvis, b.hasButtR);
        drawLine(b.buttR, b.kneeR, b.hasButtR, b.hasKneeR);
        drawLine(b.kneeR, b.footR, b.hasKneeR, b.hasFootR);
    }

    void DrawBox(const BoneScreen& bones, ImColor color, float& outX, float& outY, float& outW, float& outH)
    {
        float minX = FLT_MAX, minY = FLT_MAX;
        float maxX = -FLT_MAX, maxY = -FLT_MAX;
        int validBones = 0;

        auto updateBounds = [&](const glm::vec2& point, bool valid) {
            if (valid) {
                minX = (point.x < minX) ? point.x : minX;
                minY = (point.y < minY) ? point.y : minY;
                maxX = (point.x > maxX) ? point.x : maxX;
                maxY = (point.y > maxY) ? point.y : maxY;
                validBones++;
            }
        };

        updateBounds(bones.head, bones.hasHead);
        updateBounds(bones.neck, bones.hasNeck);
        updateBounds(bones.spine1, bones.hasSpine1);
        updateBounds(bones.spine2, bones.hasSpine2);
        updateBounds(bones.spine3, bones.hasSpine3);
        updateBounds(bones.pelvis, bones.hasPelvis);
        updateBounds(bones.sholL, bones.hasSholL);
        updateBounds(bones.elbowL, bones.hasElbowL);
        updateBounds(bones.wristL, bones.hasWristL);
        updateBounds(bones.sholR, bones.hasSholR);
        updateBounds(bones.elbowR, bones.hasElbowR);
        updateBounds(bones.wristR, bones.hasWristR);
        updateBounds(bones.buttL, bones.hasButtL);
        updateBounds(bones.kneeL, bones.hasKneeL);
        updateBounds(bones.footL, bones.hasFootL);
        updateBounds(bones.buttR, bones.hasButtR);
        updateBounds(bones.kneeR, bones.hasKneeR);
        updateBounds(bones.footR, bones.hasFootR);

        if (bones.hasHead && validBones > 0) {
            float headHeight = (maxY - minY) * 0.15f;
            minY -= headHeight;
        }

        if (validBones > 0) {
            float padding = 5.0f;
            outX = minX - padding;
            outY = minY - padding;
            outW = (maxX - minX) + (padding * 2);
            outH = (maxY - minY) + (padding * 2);

            ImDrawList* draw = ImGui::GetBackgroundDrawList();
            float thickness = 1.5f;
            float cornerSize = outW * 0.2f;
            if (cornerSize > 15.0f) cornerSize = 15.0f;

            switch (sdk::espBoxStyle) {
            case 0:
            {
                // Top-left corner
                draw->AddLine(ImVec2(outX, outY), ImVec2(outX + cornerSize, outY), color, thickness);
                draw->AddLine(ImVec2(outX, outY), ImVec2(outX, outY + cornerSize), color, thickness);
                // Top-right corner
                draw->AddLine(ImVec2(outX + outW, outY), ImVec2(outX + outW - cornerSize, outY), color, thickness);
                draw->AddLine(ImVec2(outX + outW, outY), ImVec2(outX + outW, outY + cornerSize), color, thickness);
                // Bottom-left corner
                draw->AddLine(ImVec2(outX, outY + outH), ImVec2(outX + cornerSize, outY + outH), color, thickness);
                draw->AddLine(ImVec2(outX, outY + outH), ImVec2(outX, outY + outH - cornerSize), color, thickness);
                // Bottom-right corner
                draw->AddLine(ImVec2(outX + outW, outY + outH), ImVec2(outX + outW - cornerSize, outY + outH), color, thickness);
                draw->AddLine(ImVec2(outX + outW, outY + outH), ImVec2(outX + outW, outY + outH - cornerSize), color, thickness);
                break;
            }
            case 1:
            {
                draw->AddRect(ImVec2(outX, outY), ImVec2(outX + outW, outY + outH), color, 0.0f, 0, thickness);
                break;
            }
            case 2:
            {
                draw->AddRect(ImVec2(outX, outY), ImVec2(outX + outW, outY + outH), color, 8.0f, 0, thickness);
                break;
            }
            case 3:
            {
                ImColor fillColor(color.Value.x, color.Value.y, color.Value.z, 0.25f);
                draw->AddRectFilled(ImVec2(outX, outY), ImVec2(outX + outW, outY + outH), fillColor, 4.0f);
                draw->AddRect(ImVec2(outX, outY), ImVec2(outX + outW, outY + outH), color, 4.0f, 0, thickness);
                break;
            }
            case 4:
            {
                if (!bones.has3D || !bones.hasPelvis) {
                    draw->AddRect(ImVec2(outX, outY), ImVec2(outX + outW, outY + outH), color, 0.0f, 0, thickness);
                    break;
                }
                
                float minX = bones.pelvis3D.x, maxX = bones.pelvis3D.x;
                float minY = bones.pelvis3D.y, maxY = bones.pelvis3D.y;
                float minZ = bones.pelvis3D.z, maxZ = bones.pelvis3D.z;
                
                if (bones.hasHead) {
                    minX = (bones.head3D.x < minX) ? bones.head3D.x : minX; maxX = (bones.head3D.x > maxX) ? bones.head3D.x : maxX;
                    minY = (bones.head3D.y < minY) ? bones.head3D.y : minY; maxY = (bones.head3D.y > maxY) ? bones.head3D.y : maxY;
                    minZ = (bones.head3D.z < minZ) ? bones.head3D.z : minZ; maxZ = (bones.head3D.z > maxZ) ? bones.head3D.z : maxZ;
                }
                if (bones.hasFootL) {
                    minX = (bones.footL3D.x < minX) ? bones.footL3D.x : minX; maxX = (bones.footL3D.x > maxX) ? bones.footL3D.x : maxX;
                    minY = (bones.footL3D.y < minY) ? bones.footL3D.y : minY; maxY = (bones.footL3D.y > maxY) ? bones.footL3D.y : maxY;
                    minZ = (bones.footL3D.z < minZ) ? bones.footL3D.z : minZ; maxZ = (bones.footL3D.z > maxZ) ? bones.footL3D.z : maxZ;
                }
                if (bones.hasFootR) {
                    minX = (bones.footR3D.x < minX) ? bones.footR3D.x : minX; maxX = (bones.footR3D.x > maxX) ? bones.footR3D.x : maxX;
                    minY = (bones.footR3D.y < minY) ? bones.footR3D.y : minY; maxY = (bones.footR3D.y > maxY) ? bones.footR3D.y : maxY;
                    minZ = (bones.footR3D.z < minZ) ? bones.footR3D.z : minZ; maxZ = (bones.footR3D.z > maxZ) ? bones.footR3D.z : maxZ;
                }
                
                float width = (maxX - minX) * 1.5f;
                float centerX = (minX + maxX) * 0.5f;
                minX = centerX - width * 0.5f;
                maxX = centerX + width * 0.5f;
                
                float height = maxY - minY;
                maxY = minY + height * 1.25f;
                
                math::Vec3 corners3D[8];
                corners3D[0] = math::Vec3(minX, minY, minZ);
                corners3D[1] = math::Vec3(maxX, minY, minZ);
                corners3D[2] = math::Vec3(maxX, minY, maxZ);
                corners3D[3] = math::Vec3(minX, minY, maxZ);
                corners3D[4] = math::Vec3(minX, maxY, minZ);
                corners3D[5] = math::Vec3(maxX, maxY, minZ);
                corners3D[6] = math::Vec3(maxX, maxY, maxZ);
                corners3D[7] = math::Vec3(minX, maxY, maxZ);
                
                glm::vec2 corners2D[8];
                bool validCorners[8];
                
                for (int i = 0; i < 8; i++) {
                    validCorners[i] = false;
                    corners2D[i] = glm::vec2(0, 0);
                }
                
                for (int i = 0; i < 8; i++) {
                    glm::vec3 worldPos(corners3D[i].x, corners3D[i].y, corners3D[i].z);
                    validCorners[i] = w2s(bloodstrike::renderer::camera, worldPos, corners2D[i], false);
                }
                
                int validCount = 0;
                for (int i = 0; i < 8; i++) if (validCorners[i]) validCount++;
                
                if (validCount < 4) {
                    draw->AddRect(ImVec2(outX, outY), ImVec2(outX + outW, outY + outH), color, 0.0f, 0, thickness);
                    break;
                }
                
                if (validCorners[0] && validCorners[1]) draw->AddLine(ImVec2(corners2D[0].x, corners2D[0].y), ImVec2(corners2D[1].x, corners2D[1].y), color, thickness);
                if (validCorners[1] && validCorners[2]) draw->AddLine(ImVec2(corners2D[1].x, corners2D[1].y), ImVec2(corners2D[2].x, corners2D[2].y), color, thickness);
                if (validCorners[2] && validCorners[3]) draw->AddLine(ImVec2(corners2D[2].x, corners2D[2].y), ImVec2(corners2D[3].x, corners2D[3].y), color, thickness);
                if (validCorners[3] && validCorners[0]) draw->AddLine(ImVec2(corners2D[3].x, corners2D[3].y), ImVec2(corners2D[0].x, corners2D[0].y), color, thickness);
                
                if (validCorners[4] && validCorners[5]) draw->AddLine(ImVec2(corners2D[4].x, corners2D[4].y), ImVec2(corners2D[5].x, corners2D[5].y), color, thickness);
                if (validCorners[5] && validCorners[6]) draw->AddLine(ImVec2(corners2D[5].x, corners2D[5].y), ImVec2(corners2D[6].x, corners2D[6].y), color, thickness);
                if (validCorners[6] && validCorners[7]) draw->AddLine(ImVec2(corners2D[6].x, corners2D[6].y), ImVec2(corners2D[7].x, corners2D[7].y), color, thickness);
                if (validCorners[7] && validCorners[4]) draw->AddLine(ImVec2(corners2D[7].x, corners2D[7].y), ImVec2(corners2D[4].x, corners2D[4].y), color, thickness);
                
                for (int i = 0; i < 4; i++) {
                    if (validCorners[i] && validCorners[i+4]) {
                        draw->AddLine(ImVec2(corners2D[i].x, corners2D[i].y), ImVec2(corners2D[i+4].x, corners2D[i+4].y), color, thickness);
                    }
                }
                
                if (validCorners[2] && validCorners[3] && validCorners[7] && validCorners[6]) {
                    ImColor backFill(color.Value.x, color.Value.y, color.Value.z, 0.15f);
                    ImVec2 backFace[4] = {
                        ImVec2(corners2D[2].x, corners2D[2].y),
                        ImVec2(corners2D[3].x, corners2D[3].y),
                        ImVec2(corners2D[7].x, corners2D[7].y),
                        ImVec2(corners2D[6].x, corners2D[6].y)
                    };
                    draw->AddConvexPolyFilled(backFace, 4, backFill);
                }
                
                if (validCorners[0] && validCorners[1] && validCorners[5] && validCorners[4]) {
                    ImColor frontFill(color.Value.x, color.Value.y, color.Value.z, 0.25f);
                    ImVec2 frontFace[4] = {
                        ImVec2(corners2D[0].x, corners2D[0].y),
                        ImVec2(corners2D[1].x, corners2D[1].y),
                        ImVec2(corners2D[5].x, corners2D[5].y),
                        ImVec2(corners2D[4].x, corners2D[4].y)
                    };
                    draw->AddConvexPolyFilled(frontFace, 4, frontFill);
                }
                
                break;
            }
            }
        }
    }

    void DrawTracers(float targetX, float targetY, bool useBox, float boxX, float boxY, float boxW, float boxH, ImColor color)
    {
        ImVec2 screenBottom(ImGui::GetIO().DisplaySize.x / 2.0f, ImGui::GetIO().DisplaySize.y - 50.0f);
        ImVec2 lineEnd;

        if (useBox) {
            lineEnd = ImVec2(boxX + (boxW / 2.0f), boxY + boxH);
        } else {
            lineEnd = ImVec2(targetX, targetY);
        }

        ImGui::GetBackgroundDrawList()->AddLine(screenBottom, lineEnd, color, 1.5f);
    }

    void DrawDistance(float distance, float targetX, float targetY, bool useBox, float boxX, float boxY, float boxW, float boxH)
    {
        std::string dist_txt = std::format("{:.0f}m", distance);

        ImVec2 textPos;
        if (useBox) {
            textPos = ImVec2(boxX + (boxW / 2.0f) - 15, boxY + boxH + 5.0f);
        } else {
            textPos = ImVec2(targetX + 8.0f, targetY - 6.0f);
        }

        ImU32 textColor = IM_COL32((int)(colors::espDistance[0] * 255.0f), (int)(colors::espDistance[1] * 255.0f), (int)(colors::espDistance[2] * 255.0f), 255);
        ImU32 outlineColor = IM_COL32(0, 0, 0, 255);

        ImGui::GetBackgroundDrawList()->AddText(ImVec2(textPos.x - 1, textPos.y - 1), outlineColor, dist_txt.c_str());
        ImGui::GetBackgroundDrawList()->AddText(ImVec2(textPos.x + 1, textPos.y - 1), outlineColor, dist_txt.c_str());
        ImGui::GetBackgroundDrawList()->AddText(ImVec2(textPos.x - 1, textPos.y + 1), outlineColor, dist_txt.c_str());
        ImGui::GetBackgroundDrawList()->AddText(ImVec2(textPos.x + 1, textPos.y + 1), outlineColor, dist_txt.c_str());
        ImGui::GetBackgroundDrawList()->AddText(ImVec2(textPos.x, textPos.y - 1), outlineColor, dist_txt.c_str());
        ImGui::GetBackgroundDrawList()->AddText(ImVec2(textPos.x, textPos.y + 1), outlineColor, dist_txt.c_str());
        ImGui::GetBackgroundDrawList()->AddText(ImVec2(textPos.x - 1, textPos.y), outlineColor, dist_txt.c_str());
        ImGui::GetBackgroundDrawList()->AddText(ImVec2(textPos.x + 1, textPos.y), outlineColor, dist_txt.c_str());
        ImGui::GetBackgroundDrawList()->AddText(textPos, textColor, dist_txt.c_str());
    }

    void DrawChapeauChinois3D(const BoneScreen& bones, ImColor color)
    {
        ImDrawList* draw = ImGui::GetBackgroundDrawList();
        if (!bones.hasNeck || !bones.hasHead) return;
        
        float neckX = bones.neck.x;
        float neckY = bones.neck.y;
        
        float headNeckDist = sqrtf(
            (bones.head.x - bones.neck.x) * (bones.head.x - bones.neck.x) +
            (bones.head.y - bones.neck.y) * (bones.head.y - bones.neck.y)
        );
        
        float baseRadius = headNeckDist * 3.0f;
        float height = headNeckDist * 1.5f;
        float neckOffset = headNeckDist * 1.0f;
        
        ImVec2 top(neckX, neckY - height - neckOffset);
        
        const int segments = 16;
        ImVec2 basePoints[segments];
        
        float rotationAngle = 0.0f;
        if (sdk::espChapeauRotationSpeed > 0.0f) {
            rotationAngle = ImGui::GetTime() * sdk::espChapeauRotationSpeed * 2.0f;
        }
        
        for (int i = 0; i < segments; i++) {
            float angle = (float)i / (float)segments * 6.28318f + rotationAngle;
            basePoints[i] = ImVec2(
                neckX + cosf(angle) * baseRadius,
                neckY + sinf(angle) * baseRadius * 0.25f
            );
        }
        
        ImColor fillColor(color.Value.x, color.Value.y, color.Value.z, 0.25f);
        ImColor darkColor(
            (int)(color.Value.x * 150), 
            (int)(color.Value.y * 150), 
            (int)(color.Value.z * 150), 
            140
        );
        
        for (int i = 0; i < segments; i++) {
            int next = (i + 1) % segments;
            ImColor faceColor = (i % 2 == 0) ? fillColor : darkColor;
            draw->AddTriangleFilled(basePoints[i], basePoints[next], top, faceColor);
        }
        
        for (int i = 0; i < segments; i++) {
            int next = (i + 1) % segments;
            draw->AddLine(basePoints[i], basePoints[next], color, 1.5f);
        }
        
        for (int i = 0; i < segments; i += 2) {
            draw->AddLine(basePoints[i], top, color, 1.0f);
        }
        
        draw->AddCircleFilled(top, 3.0f, color, 8);
    }

    void DrawChapeauChinoisTrue3D(glm::vec3 neckPos, glm::vec3 headPos, uint64_t camera, ImColor color)
    {
        ImDrawList* draw = ImGui::GetBackgroundDrawList();
        if (!camera) return;

        float headNeckDist = glm::distance(headPos, neckPos);
        float baseRadius = headNeckDist * 2.0f;
        float coneHeight = headNeckDist * 1.5f;

        glm::vec3 baseCenter = neckPos + glm::vec3(0, headNeckDist * 0.5f, 0);
        glm::vec3 apexPos = baseCenter + glm::vec3(0, coneHeight, 0);

        const int segments = 16;
        glm::vec3 basePoints3D[segments];
        glm::vec2 basePointsScreen[segments];
        glm::vec2 apexScreen;

        float rotationAngle = 0.0f;
        if (sdk::espChapeauRotationSpeed > 0.0f) {
            rotationAngle = ImGui::GetTime() * sdk::espChapeauRotationSpeed * 2.0f;
        }

        for (int i = 0; i < segments; i++) {
            float angle = (float)i / (float)segments * 6.28318f + rotationAngle;
            float cosA = cosf(angle);
            float sinA = sinf(angle);

            basePoints3D[i] = baseCenter + glm::vec3(cosA * baseRadius, 0, sinA * baseRadius);

            glm::vec2 screenPos;
            if (w2s(camera, basePoints3D[i], screenPos, false)) {
                basePointsScreen[i] = screenPos;
            } else {
                basePointsScreen[i] = glm::vec2(-1, -1);
            }
        }

        if (!w2s(camera, apexPos, apexScreen, false)) return;

        ImColor fillColor(color.Value.x, color.Value.y, color.Value.z, 0.25f);
        ImColor darkColor(
            (int)(color.Value.x * 150),
            (int)(color.Value.y * 150),
            (int)(color.Value.z * 150),
            140
        );

        for (int i = 0; i < segments; i++) {
            int next = (i + 1) % segments;
            if (basePointsScreen[i].x < 0 || basePointsScreen[next].x < 0) continue;

            ImColor faceColor = (i % 2 == 0) ? fillColor : darkColor;
            draw->AddTriangleFilled(
                ImVec2(basePointsScreen[i].x, basePointsScreen[i].y),
                ImVec2(basePointsScreen[next].x, basePointsScreen[next].y),
                ImVec2(apexScreen.x, apexScreen.y),
                faceColor
            );
        }

        for (int i = 0; i < segments; i++) {
            int next = (i + 1) % segments;
            if (basePointsScreen[i].x < 0 || basePointsScreen[next].x < 0) continue;

            draw->AddLine(
                ImVec2(basePointsScreen[i].x, basePointsScreen[i].y),
                ImVec2(basePointsScreen[next].x, basePointsScreen[next].y),
                color, 1.5f
            );
        }

        for (int i = 0; i < segments; i += 2) {
            if (basePointsScreen[i].x < 0) continue;
            draw->AddLine(
                ImVec2(basePointsScreen[i].x, basePointsScreen[i].y),
                ImVec2(apexScreen.x, apexScreen.y),
                color, 1.0f
            );
        }

        draw->AddCircleFilled(ImVec2(apexScreen.x, apexScreen.y), 3.0f, color, 8);
    }

    void DrawFovCircle()
    {
        if (!sdk::aimbotDrawFov) return;

        ImVec2 screenCenter(ImGui::GetIO().DisplaySize.x / 2.0f, ImGui::GetIO().DisplaySize.y / 2.0f);
        bool isKeyPressed = (GetAsyncKeyState(sdk::aimbotKey) & 0x8000) != 0;

        float* fovColor = isKeyPressed ? colors::fovActive : colors::fovInactive;
        ImColor circleColor((int)(fovColor[0] * 255.0f), (int)(fovColor[1] * 255.0f), (int)(fovColor[2] * 255.0f), (int)(fovColor[3] * 255.0f));

        ImGui::GetBackgroundDrawList()->AddCircle(screenCenter, sdk::aimbotFov, circleColor, 64, 1.5f);

        if (isKeyPressed) {
            ImGui::GetBackgroundDrawList()->AddCircle(
                screenCenter, sdk::aimbotFov + 2,
                ImColor((int)(fovColor[0] * 255.0f), (int)(fovColor[1] * 255.0f), (int)(fovColor[2] * 255.0f), 60), 64, 2.0f);
        }
    }

    void DrawRadar()
    {
        if (!sdk::espRadar) return;

        glm::vec3 localPos = get_local_position();

        ImDrawList* draw = ImGui::GetBackgroundDrawList();
        float radarSize = sdk::espRadarSize;
        if (radarSize < 80.0f) radarSize = 80.0f;
        if (radarSize > 350.0f) radarSize = 350.0f;
        sdk::espRadarSize = radarSize;

        float radarZoom = sdk::espRadarZoom;
        if (radarZoom < 0.20f) radarZoom = 0.20f;
        if (radarZoom > 4.00f) radarZoom = 4.00f;
        sdk::espRadarZoom = radarZoom;

        float radarEnemySize = sdk::espRadarEnemySize;
        if (radarEnemySize < 1.0f) radarEnemySize = 1.0f;
        if (radarEnemySize > 8.0f) radarEnemySize = 8.0f;
        sdk::espRadarEnemySize = radarEnemySize;

        const float padding = 24.0f;
        const ImVec2 displaySize = ImGui::GetIO().DisplaySize;

        if (sdk::espRadarPosX < 0.0f || sdk::espRadarPosY < 0.0f) {
            sdk::espRadarPosX = padding;
            sdk::espRadarPosY = displaySize.y - radarSize - padding;
        }

        static bool draggingRadar = false;
        static ImVec2 dragOffset(0.0f, 0.0f);

        if (menu::show) {
            ImVec2 mousePos = ImGui::GetIO().MousePos;
            ImVec2 preMin(sdk::espRadarPosX, sdk::espRadarPosY);
            ImVec2 preMax(preMin.x + radarSize, preMin.y + radarSize);
            bool hovered = (mousePos.x >= preMin.x && mousePos.x <= preMax.x && mousePos.y >= preMin.y && mousePos.y <= preMax.y);

            if (!draggingRadar && hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                draggingRadar = true;
                dragOffset = ImVec2(mousePos.x - preMin.x, mousePos.y - preMin.y);
            }

            if (draggingRadar) {
                if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                    sdk::espRadarPosX = mousePos.x - dragOffset.x;
                    sdk::espRadarPosY = mousePos.y - dragOffset.y;
                } else {
                    draggingRadar = false;
                }
            }
        } else {
            draggingRadar = false;
        }

        float maxRadarX = displaySize.x - radarSize - 1.0f;
        float maxRadarY = displaySize.y - radarSize - 1.0f;
        if (maxRadarX < 0.0f) maxRadarX = 0.0f;
        if (maxRadarY < 0.0f) maxRadarY = 0.0f;

        if (sdk::espRadarPosX < 0.0f) sdk::espRadarPosX = 0.0f;
        else if (sdk::espRadarPosX > maxRadarX) sdk::espRadarPosX = maxRadarX;

        if (sdk::espRadarPosY < 0.0f) sdk::espRadarPosY = 0.0f;
        else if (sdk::espRadarPosY > maxRadarY) sdk::espRadarPosY = maxRadarY;

        ImVec2 radarMin(sdk::espRadarPosX, sdk::espRadarPosY);
        ImVec2 radarMax(radarMin.x + radarSize, radarMin.y + radarSize);
        ImVec2 radarCenter((radarMin.x + radarMax.x) * 0.5f, (radarMin.y + radarMax.y) * 0.5f);
        const float radarRadius = (radarSize * 0.5f) - 10.0f;
        float radarRange = (sdk::espMaxDistance > 50.0f) ? sdk::espMaxDistance : 50.0f;
        radarRange /= radarZoom;
        if (radarRange < 10.0f) radarRange = 10.0f;
        const float worldToRadar = radarRadius / radarRange;

        auto MenuColor = [&](const float col[4], float alphaMul) -> ImU32
        {
            float alpha = col[3] * menu::alpha * alphaMul;
            if (alpha < 0.0f) alpha = 0.0f;
            if (alpha > 1.0f) alpha = 1.0f;
            return IM_COL32(
                (int)(col[0] * 255.0f),
                (int)(col[1] * 255.0f),
                (int)(col[2] * 255.0f),
                (int)(alpha * 255.0f)
            );
        };

        ImU32 bgColor = MenuColor(menu::bgColor, 0.95f);
        ImU32 titleBgColor = MenuColor(menu::titleBg, 0.95f);
        ImU32 borderColor = MenuColor(menu::accentColor, 0.70f);
        ImU32 axisColor = MenuColor(menu::accentColor, 0.20f);
        ImU32 localDotColor = MenuColor(menu::accentColor, 1.00f);
        ImU32 titleTextColor = MenuColor(menu::accentColor, 1.00f);
        ImU32 infoTextColor = MenuColor(menu::accentColor, 0.75f);

        draw->AddRectFilled(radarMin, radarMax, bgColor, 8.0f);
        draw->AddRectFilled(radarMin, ImVec2(radarMax.x, radarMin.y + 22.0f), titleBgColor, 8.0f);
        draw->AddRect(radarMin, radarMax, borderColor, 8.0f, 0, 1.3f);
        draw->AddLine(ImVec2(radarCenter.x, radarMin.y + 8.0f), ImVec2(radarCenter.x, radarMax.y - 8.0f), axisColor, 1.0f);
        draw->AddLine(ImVec2(radarMin.x + 8.0f, radarCenter.y), ImVec2(radarMax.x - 8.0f, radarCenter.y), axisColor, 1.0f);
        draw->AddText(ImVec2(radarMin.x + 8.0f, radarMin.y + 4.0f), titleTextColor, "RADAR");
        draw->AddCircleFilled(radarCenter, 3.0f, localDotColor, 12);
        if (menu::show) {
            draw->AddText(ImVec2(radarMin.x + 8.0f, radarMax.y - 20.0f), infoTextColor, "Drag");
        }

        ImColor enemyColor((int)(colors::espBox[0] * 255.0f), (int)(colors::espBox[1] * 255.0f), (int)(colors::espBox[2] * 255.0f), 255);
        ImColor botColor((int)(colors::espBots[0] * 255.0f), (int)(colors::espBots[1] * 255.0f), (int)(colors::espBots[2] * 255.0f), 255);
        int plottedEnemies = 0;

        for (const auto& radarEntry : g_radarEntries) {
            if (sdk::espHideBots && radarEntry.isBot) continue;

            // Ground plane for radar: X/Z (Y is vertical height)
            float dx = radarEntry.pos.x - localPos.x;
            float dz = radarEntry.pos.z - localPos.z;
            if (!std::isfinite(dx) || !std::isfinite(dz)) continue;

            float distance2D = std::sqrt((dx * dx) + (dz * dz));
            if (distance2D > radarRange) continue;

            float radarX = dx * worldToRadar;
            float radarY = dz * worldToRadar;
            float radarDist = std::sqrt((radarX * radarX) + (radarY * radarY));
            if (radarDist > radarRadius) {
                float clamp = radarRadius / radarDist;
                radarX *= clamp;
                radarY *= clamp;
            }

            // Rotate radar 90 degrees to the left
            float rotatedX = -radarY;
            float rotatedY = radarX;
            // Mirror horizontally: left <-> right
            ImVec2 dotPos(radarCenter.x - rotatedX, radarCenter.y - rotatedY);
            ImColor dotColor = (sdk::espShowBots && radarEntry.isBot) ? botColor : enemyColor;
            draw->AddCircleFilled(dotPos, radarEnemySize, dotColor, 10);
            plottedEnemies++;
        }

        char radarCount[32];
        sprintf_s(radarCount, "E:%d", plottedEnemies);
        draw->AddText(ImVec2(radarMax.x - 42.0f, radarMin.y + 4.0f), titleTextColor, radarCount);
    }

    void RenderPlayers()
    {
        if (!sdk::espSkeleton && !sdk::espBox && !sdk::espDistance && !sdk::espTracers && !sdk::espChapeauChinois && !sdk::espRadar) return;

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

        // FIX: lire le teamID en int (4 octets) et non uint64_t (8 octets)
        int localTeam = 0;
        if (sdk::aimbotTeamCheck && bloodstrike::renderer::localActor)
        {
            localTeam = memory->read<int>(bloodstrike::renderer::localActor + offsets::player::teamID);
        }

        // Read local position fresh right before loop to minimize latency
        glm::vec3 local_pos = get_local_position();

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

                DirectX::XMFLOAT3X4 transform = memory->read<DirectX::XMFLOAT3X4>(IEntity + offsets::entity::transform);
                bool isBot = IsBot(IEntity, actorProps);
                if (sdk::espHideBots && isBot) { currentActor = memory->read<uint64_t>(currentActor); continue; }

                if (sdk::espRadar) {
                    glm::vec3 radarPos(transform._32, transform._33, transform._34);
                    float radarDist = glm::distance(radarPos, local_pos);
                    if (radarDist <= sdk::espMaxDistance) {
                        RadarAddEntity(IEntity, radarPos, isBot);
                    }
                }

                BoneScreen bones;
                if (GetBonesForPlayer(BipedPose, transform, bones))
                {
                    glm::vec3 headPos;
                    glm::vec2 headScreen;
                    if (ReadBonePos3D(BipedPose, transform, offsets::bone::head, headPos, headScreen))
                    {
                        float dist = glm::distance(headPos, local_pos);
                        if (dist > sdk::espMaxDistance) { currentActor = memory->read<uint64_t>(currentActor); continue; }
                        if (sdk::espRadar) {
                            RadarAddEntity(IEntity, headPos, isBot);
                        }
                     
                    ImColor skeletonColor((int)(colors::espBones[0] * 255.0f), (int)(colors::espBones[1] * 255.0f), (int)(colors::espBones[2] * 255.0f), (int)(colors::espBones[3] * 255.0f));
                    ImColor tracerColor((int)(colors::espLines[0] * 255.0f), (int)(colors::espLines[1] * 255.0f), (int)(colors::espLines[2] * 255.0f), (int)(colors::espLines[3] * 255.0f));
                    
                    ImColor boxColor;
                    if (sdk::espShowBots && isBot) {
                        skeletonColor = ImColor((int)(colors::espBonesBot[0] * 255.0f), (int)(colors::espBonesBot[1] * 255.0f), (int)(colors::espBonesBot[2] * 255.0f), (int)(colors::espBonesBot[3] * 255.0f));
                        boxColor = ImColor((int)(colors::espBots[0] * 255.0f), (int)(colors::espBots[1] * 255.0f), (int)(colors::espBots[2] * 255.0f), (int)(colors::espBots[3] * 255.0f));
                        tracerColor = ImColor((int)(colors::espLinesBot[0] * 255.0f), (int)(colors::espLinesBot[1] * 255.0f), (int)(colors::espLinesBot[2] * 255.0f), (int)(colors::espLinesBot[3] * 255.0f));
                    } else {
                        boxColor = ImColor((int)(colors::espBox[0] * 255.0f), (int)(colors::espBox[1] * 255.0f), (int)(colors::espBox[2] * 255.0f), (int)(colors::espBox[3] * 255.0f));
                    }

                        float boxX = 0, boxY = 0, boxW = 0, boxH = 0;
                        bool hasBox = false;

                        if (sdk::espSkeleton)
                        {
                            DrawSkeleton(bones, skeletonColor);
                        }

                        if (sdk::espBox)
                        {
                            DrawBox(bones, boxColor, boxX, boxY, boxW, boxH);
                            hasBox = true;
                        }

                        if (sdk::espTracers)
                        {
                            DrawTracers(headScreen.x, headScreen.y, hasBox, boxX, boxY, boxW, boxH, tracerColor);
                        }

                        if (sdk::espDistance)
                        {
                            DrawDistance(dist, headScreen.x, headScreen.y, hasBox, boxX, boxY, boxW, boxH);
                        }

                        if (sdk::espChapeauChinois && bones.hasNeck && bones.hasHead)
                        {
                            ImColor chapeauColor((int)(colors::espChapeau[0] * 255.0f), (int)(colors::espChapeau[1] * 255.0f), (int)(colors::espChapeau[2] * 255.0f), (int)(colors::espChapeau[3] * 255.0f));
                            DrawChapeauChinoisTrue3D(bones.neck3D, bones.head3D, bloodstrike::renderer::camera, chapeauColor);
                        }
                    }
                }

                currentActor = memory->read<uint64_t>(currentActor);
            } while (currentActor != head && currentActor != 0);
        }
    }

    void Render()
    {
        DrawFovCircle();
        RadarBeginFrame();
        RenderPlayers();              // EntityList method (good for close range)
        RenderPlayersViaSingleton();  // Singleton method (may have different entities)
        DrawRadar();
        RenderLoot();                 // Loot/items ESP
        TestObjectListDiscovery();    // Debug: test object list chain
        DebugEntityScan();            // Debug: scan all entity types to find loot
    }

    void RenderPlayersViaSingleton()
    {
        if (!sdk::espSkeleton && !sdk::espBox && !sdk::espDistance && !sdk::espTracers && !sdk::espChapeauChinois && !sdk::espRadar) return;

        uint64_t base = memory->get_module_address();

        uint64_t ientityMgr = memory->read<uint64_t>(base + offsets::singleton::MessiahIEntity);
        if (!ientityMgr) return;

        uint64_t entityArray = memory->read<uint64_t>(ientityMgr + 0x0);
        uint64_t entityCount = memory->read<uint64_t>(ientityMgr + 0x8);

        if (!entityArray || entityCount == 0) return;

        uint64_t localActor = 0;
        uint64_t camera = 0;

        uint64_t icamera = memory->read<uint64_t>(base + offsets::singleton::MessiahICamera);
        if (icamera) {
            camera = memory->read<uint64_t>(icamera + 0x0);
        }

        if (!camera) {
            uint64_t ClientEngine = memory->read<uint64_t>(base + offsets::module::ClientEngineSteam);
            if (ClientEngine) {
                uint64_t IGameplay = memory->read<uint64_t>(ClientEngine + offsets::player::IGameplay);
                if (IGameplay) {
                    uint64_t ClientPlayer = memory->read<uint64_t>(IGameplay + offsets::player::ClientPlayer);
                    if (ClientPlayer) {
                        camera = memory->read<uint64_t>(ClientPlayer + offsets::player::camera);
                        localActor = memory->read<uint64_t>(ClientPlayer + offsets::player::localActor);
                    }
                }
            }
        }

        if (!camera) return;
        bloodstrike::renderer::camera = camera;
        bloodstrike::renderer::localActor = localActor;

        // FIX: lire le teamID en int (4 octets) et non uint64_t (8 octets)
        int localTeam = 0;
        if (sdk::aimbotTeamCheck && localActor) {
            localTeam = memory->read<int>(localActor + offsets::player::teamID);
        }

        // Read local position fresh right before loop to minimize latency
        glm::vec3 local_pos = get_local_position();

        for (uint64_t i = 0; i < entityCount && i < 100; i++) {
            uint64_t entityPtr = memory->read<uint64_t>(entityArray + i * 0x8);
            if (!entityPtr) continue;

            uint64_t IEntity = entityPtr;

            uint64_t entityMask = memory->read<uint64_t>(IEntity + offsets::entity::entityMask);
            if (entityMask != 2) continue;

            if (IEntity == localActor) continue;

            int theirTeam = memory->read<int>(IEntity + offsets::player::teamID);
            if (sdk::aimbotTeamCheck && localTeam != 0 && theirTeam == localTeam) continue;

            uint64_t IArea = memory->read<uint64_t>(IEntity + offsets::entity::IArea);
            if (IArea == 0x0) continue;

            DirectX::XMFLOAT3X4 transform = memory->read<DirectX::XMFLOAT3X4>(IEntity + offsets::entity::transform);
            uint64_t actorProps = memory->read<uint64_t>(IEntity + offsets::entity::actorProps);
            if (!actorProps) {
                uint64_t actorComponent = memory->read<uint64_t>(IEntity + offsets::entity::actorComponent);
                if (actorComponent) {
                    actorProps = memory->read<uint64_t>(actorComponent + offsets::entity::actorProps);
                }
            }

            bool isBot = IsBot(IEntity, actorProps);
            if (sdk::espHideBots && isBot) continue;

            if (sdk::espRadar) {
                glm::vec3 radarPos(transform._32, transform._33, transform._34);
                float radarDist = glm::distance(radarPos, local_pos);
                if (radarDist <= sdk::espMaxDistance) {
                    RadarAddEntity(IEntity, radarPos, isBot);
                }
            }

            uint64_t BipedPose = 0;

            uint64_t skelComp = memory->read<uint64_t>(base + offsets::singleton::MessiahSkeletonComponent);
            if (skelComp) {
                BipedPose = memory->read<uint64_t>(IEntity + offsets::entity::pose + offsets::entity::BipedPose);
            }

            if (!BipedPose) {
                uint64_t actorComponent = memory->read<uint64_t>(IEntity + offsets::entity::actorComponent);
                if (actorComponent) {
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

            BoneScreen bones;
            if (GetBonesForPlayer(BipedPose, transform, bones)) {
                glm::vec3 headPos;
                glm::vec2 headScreen;
                if (ReadBonePos3D(BipedPose, transform, offsets::bone::head, headPos, headScreen)) {
                    float dist = glm::distance(headPos, local_pos);
                    if (dist > sdk::espMaxDistance) continue;
                    if (sdk::espRadar) {
                        RadarAddEntity(IEntity, headPos, isBot);
                    }

                    ImColor skeletonColor((int)(colors::espBones[0] * 255.0f), (int)(colors::espBones[1] * 255.0f), (int)(colors::espBones[2] * 255.0f), (int)(colors::espBones[3] * 255.0f));
                    ImColor boxColor((int)(colors::espBox[0] * 255.0f), (int)(colors::espBox[1] * 255.0f), (int)(colors::espBox[2] * 255.0f), (int)(colors::espBox[3] * 255.0f));
                    ImColor tracerColor((int)(colors::espLines[0] * 255.0f), (int)(colors::espLines[1] * 255.0f), (int)(colors::espLines[2] * 255.0f), (int)(colors::espLines[3] * 255.0f));
                    
                    // Use bot color if showing bots differently
                    if (sdk::espShowBots && isBot) {
                        skeletonColor = ImColor((int)(colors::espBonesBot[0] * 255.0f), (int)(colors::espBonesBot[1] * 255.0f), (int)(colors::espBonesBot[2] * 255.0f), (int)(colors::espBonesBot[3] * 255.0f));
                        boxColor = ImColor((int)(colors::espBots[0] * 255.0f), (int)(colors::espBots[1] * 255.0f), (int)(colors::espBots[2] * 255.0f), (int)(colors::espBots[3] * 255.0f));
                        tracerColor = ImColor((int)(colors::espLinesBot[0] * 255.0f), (int)(colors::espLinesBot[1] * 255.0f), (int)(colors::espLinesBot[2] * 255.0f), (int)(colors::espLinesBot[3] * 255.0f));
                    }

                    float boxX = 0, boxY = 0, boxW = 0, boxH = 0;
                    bool hasBox = false;

                    if (sdk::espSkeleton) {
                        DrawSkeleton(bones, skeletonColor);
                    }

                    if (sdk::espBox) {
                        DrawBox(bones, boxColor, boxX, boxY, boxW, boxH);
                        hasBox = true;
                    }

                    if (sdk::espTracers) {
                        DrawTracers(headScreen.x, headScreen.y, hasBox, boxX, boxY, boxW, boxH, tracerColor);
                    }

                    if (sdk::espDistance) {
                        DrawDistance(dist, headScreen.x, headScreen.y, hasBox, boxX, boxY, boxW, boxH);
                    }

                    if (sdk::espChapeauChinois && bones.hasNeck && bones.hasHead) {
                        ImColor chapeauColor((int)(colors::espChapeau[0] * 255.0f), (int)(colors::espChapeau[1] * 255.0f), (int)(colors::espChapeau[2] * 255.0f), (int)(colors::espChapeau[3] * 255.0f));
                        DrawChapeauChinoisTrue3D(bones.neck3D, bones.head3D, camera, chapeauColor);
                    }
                }
            }
        }
    }

    void DebugEntityScan()
    {
        if (!sdk::debugEntityScan) return;

        uint64_t base = memory->get_module_address();
        uint64_t ientityMgr = memory->read<uint64_t>(base + offsets::singleton::MessiahIEntity);
        if (!ientityMgr) return;

        uint64_t entityArray = memory->read<uint64_t>(ientityMgr + 0x0);
        uint64_t entityCount = memory->read<uint64_t>(ientityMgr + 0x8);
        if (!entityArray || entityCount == 0) return;

        uint64_t icamera = memory->read<uint64_t>(base + offsets::singleton::MessiahICamera);
        uint64_t camera = 0;
        if (icamera) camera = memory->read<uint64_t>(icamera + 0x0);
        if (!camera) return;

        static std::vector<uint64_t> foundMasks;

        ImDrawList* draw = ImGui::GetBackgroundDrawList();
        float startY = 100.0f;

        draw->AddText(ImVec2(10, startY), IM_COL32(255, 255, 0, 255), "=== ENTITY SCAN ===");
        startY += 20;

        for (uint64_t i = 0; i < entityCount && i < 200; i++) {
            uint64_t IEntity = memory->read<uint64_t>(entityArray + i * 0x8);
            if (!IEntity) continue;

            uint64_t entityMask = memory->read<uint64_t>(IEntity + offsets::entity::entityMask);
            DirectX::XMFLOAT3X4 transform = memory->read<DirectX::XMFLOAT3X4>(IEntity + offsets::entity::transform);
            glm::vec3 pos(transform._32, transform._33, transform._34);

            glm::vec2 screenPos;
            if (w2s(camera, pos, screenPos, false)) {
                ImColor color;
                switch (entityMask) {
                    case 1: color = ImColor(0, 255, 0, 200); break;
                    case 2: color = ImColor(255, 0, 0, 200); break;
                    case 3: color = ImColor(0, 0, 255, 200); break;
                    case 4: color = ImColor(255, 255, 0, 200); break;
                    default: color = ImColor(200, 200, 200, 200); break;
                }

            }
        }

        draw->AddText(ImVec2(10, startY), IM_COL32(0, 255, 255, 255), "Found masks:");
        startY += 18;
        for (auto mask : foundMasks) {
            char buf[64];
            sprintf_s(buf, "Mask %llu", mask);
            draw->AddText(ImVec2(20, startY), IM_COL32(200, 200, 200, 255), buf);
            startY += 15;
        }
    }

    void RenderLoot()
    {
        if (!sdk::espLoot) return;

        uint64_t base = memory->get_module_address();

        uint64_t iarea = memory->read<uint64_t>(base + offsets::singleton::MessiahIArea);
        if (!iarea) return;

        uint64_t icamera = memory->read<uint64_t>(base + offsets::singleton::MessiahICamera);
        uint64_t camera = 0;
        if (icamera) camera = memory->read<uint64_t>(icamera + 0x0);
        if (!camera) return;

        glm::vec3 local_pos = get_local_position();

        uint64_t obj1 = memory->read<uint64_t>(iarea + offsets::loot::ptrObject1);
        if (!obj1) return;

        uint64_t obj2 = memory->read<uint64_t>(obj1 + offsets::loot::ptrObject2);
        if (!obj2) return;

        uint64_t listBegin = memory->read<uint64_t>(obj2 + 0x0);
        uint64_t listEnd = memory->read<uint64_t>(obj2 + 0x8);
        if (!listBegin || !listEnd) return;

        int count = (int)((listEnd - listBegin) / sizeof(uint64_t));
        if (count <= 0 || count > 1000) return;

        ImDrawList* draw = ImGui::GetBackgroundDrawList();
        ImColor lootColor((int)(colors::espLoot[0] * 255.0f), (int)(colors::espLoot[1] * 255.0f), (int)(colors::espLoot[2] * 255.0f), (int)(colors::espLoot[3] * 255.0f));

        int renderedLoot = 0;

        for (int i = 0; i < count && i < 500; i++) {
            uint64_t curObj = memory->read<uint64_t>(listBegin + i * sizeof(uint64_t));
            if (!curObj) continue;

            // Get entity pointer
            uint64_t entity = memory->read<uint64_t>(curObj - offsets::loot::entityOffset);
            if (!entity) continue;

            // Read object type
            int objType = memory->read<int>(entity + offsets::loot::objectType);

            // Check if it's loot (types 2, 4, 5)
            if (objType != offsets::object_types::LOOT_1 &&
                objType != offsets::object_types::LOOT_2 &&
                objType != offsets::object_types::LOOT_3) {
                continue;
            }

            // Read position
            glm::vec3 worldPos = memory->read<glm::vec3>(entity + offsets::loot::objectPosition);

            // Distance check
            float dist = glm::distance(worldPos, local_pos);
            if (dist > sdk::espLootMaxDistance) continue;

            // Project to screen
            glm::vec2 screenPos;
            if (!w2s(camera, worldPos, screenPos, false)) continue;

                float boxSize = 8.0f;
            draw->AddRectFilled(
                ImVec2(screenPos.x - boxSize, screenPos.y - boxSize),
                ImVec2(screenPos.x + boxSize, screenPos.y + boxSize),
                lootColor, 2.0f
            );

            char distLabel[32];
            sprintf_s(distLabel, "[%.0fm]", dist);
            draw->AddText(ImVec2(screenPos.x + 12, screenPos.y - 6), lootColor, distLabel);

            const char* typeName = "Loot";
            switch (objType) {
                case 2: typeName = "Item"; break;
                case 4: typeName = "Weapon"; break;
                case 5: typeName = "Ammo"; break;
            }
            draw->AddText(ImVec2(screenPos.x + 12, screenPos.y + 8), lootColor, typeName);

            renderedLoot++;
        }

        if (sdk::debugEntityScan) {
            char debugBuf[64];
            sprintf_s(debugBuf, "Loot items: %d", renderedLoot);
            draw->AddText(ImVec2(10, 250), IM_COL32(255, 255, 0, 255), debugBuf);
        }
    }

    bool IsBot(uint64_t entity, uint64_t actorProps)
    {
        int actorType = memory->read<int>(entity + offsets::loot::actorType);
        if (actorType != offsets::object_types::HUMAN_ACTOR) {
            return true;
        }

        if (actorProps) {
        }

        uint64_t bipedPose = memory->read<uint64_t>(entity + offsets::entity::pose);
        if (!bipedPose) {
            uint64_t BipedPose = memory->read<uint64_t>(bipedPose + offsets::entity::BipedPose);
            if (!BipedPose) return true;
        }

        return false;
    }

    void TestObjectListDiscovery()
    {
        if (!sdk::debugEntityScan) return;

        uint64_t base = memory->get_module_address();
        ImDrawList* draw = ImGui::GetBackgroundDrawList();
        float y = 300.0f;

        draw->AddText(ImVec2(10, y), IM_COL32(255, 200, 0, 255), "=== OBJECT LIST TEST ===");
        y += 20;

        uint64_t iarea = memory->read<uint64_t>(base + offsets::singleton::MessiahIArea);
        char buf[128];
        sprintf_s(buf, "MessiahIArea: 0x%llX", iarea);
        draw->AddText(ImVec2(10, y), IM_COL32(200, 200, 200, 255), buf);
        y += 15;

        if (iarea) {
            uint64_t obj1 = memory->read<uint64_t>(iarea + offsets::loot::ptrObject1);
            sprintf_s(buf, "-> +0x%llX: 0x%llX", offsets::loot::ptrObject1, obj1);
            draw->AddText(ImVec2(10, y), IM_COL32(200, 200, 200, 255), buf);
            y += 15;

            if (obj1) {
                uint64_t obj2 = memory->read<uint64_t>(obj1 + offsets::loot::ptrObject2);
                sprintf_s(buf, "-> +0x%llX: 0x%llX", offsets::loot::ptrObject2, obj2);
                draw->AddText(ImVec2(10, y), IM_COL32(200, 200, 200, 255), buf);
                y += 15;
            }
        }

        uint64_t actorComp = memory->read<uint64_t>(base + offsets::singleton::MessiahActorComponent);
        sprintf_s(buf, "MessiahActorComponent: 0x%llX", actorComp);
        draw->AddText(ImVec2(10, y), IM_COL32(200, 200, 200, 255), buf);
        y += 15;

        if (offsets::loot::addrObjects != 0) {
            uint64_t directList = memory->read<uint64_t>(base + offsets::loot::addrObjects);
            sprintf_s(buf, "Direct addrObjects: 0x%llX", directList);
            draw->AddText(ImVec2(10, y), IM_COL32(200, 200, 200, 255), buf);
            y += 15;
        }
    }
}

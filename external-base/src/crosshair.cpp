#include "crosshair.hpp"
#include "settings.hpp"
#include <deps/imgui/imgui.h>
#include <cmath>
#include <Windows.h>

namespace crosshair
{
    void Render()
    {
        if (!sdk::customCrosshair) return;

        ImVec2 center(ImGui::GetIO().DisplaySize.x / 2.0f, ImGui::GetIO().DisplaySize.y / 2.0f);

        sdk::crosshairRotation += sdk::crosshairRotationSpeed * ImGui::GetIO().DeltaTime * 100.0f;
        if (sdk::crosshairRotation > 360.0f) sdk::crosshairRotation -= 360.0f;

        float size = sdk::crosshairSize;
        float thickness = sdk::crosshairThickness;
        float gap = sdk::crosshairGap;
        float rotRad = sdk::crosshairRotation * 3.14159f / 180.0f;

        bool aimbotActive = (GetAsyncKeyState(sdk::aimbotKey) & 0x8000) != 0;
        
        float* color = aimbotActive ? colors::fovActive : sdk::crosshairColor;
        ImU32 col = IM_COL32((int)(color[0] * 255),
                             (int)(color[1] * 255),
                             (int)(color[2] * 255),
                             (int)(color[3] * 255));

        auto rotatePoint = [&](ImVec2 p, float angle) -> ImVec2 {
            float s = sin(angle);
            float c = cos(angle);
            p.x -= center.x;
            p.y -= center.y;
            float xnew = p.x * c - p.y * s;
            float ynew = p.x * s + p.y * c;
            return ImVec2(xnew + center.x, ynew + center.y);
        };

        switch (sdk::crosshairStyle)
        {
        case 0:
        {
            ImVec2 pts[4] = {
                ImVec2(center.x - size - gap, center.y),
                ImVec2(center.x - gap, center.y),
                ImVec2(center.x + gap, center.y),
                ImVec2(center.x + size + gap, center.y)
            };
            for (int i = 0; i < 4; i++) pts[i] = rotatePoint(pts[i], rotRad);
            ImGui::GetBackgroundDrawList()->AddLine(pts[0], pts[1], col, thickness);
            ImGui::GetBackgroundDrawList()->AddLine(pts[2], pts[3], col, thickness);

            ImVec2 pts2[4] = {
                ImVec2(center.x, center.y - size - gap),
                ImVec2(center.x, center.y - gap),
                ImVec2(center.x, center.y + gap),
                ImVec2(center.x, center.y + size + gap)
            };
            for (int i = 0; i < 4; i++) pts2[i] = rotatePoint(pts2[i], rotRad);
            ImGui::GetBackgroundDrawList()->AddLine(pts2[0], pts2[1], col, thickness);
            ImGui::GetBackgroundDrawList()->AddLine(pts2[2], pts2[3], col, thickness);
            break;
        }
        case 1:
        {
            ImGui::GetBackgroundDrawList()->AddCircle(center, size, col, 32, thickness);
            break;
        }
        case 2:
        {
            ImGui::GetBackgroundDrawList()->AddCircle(center, size, col, 32, thickness);
            ImGui::GetBackgroundDrawList()->AddLine(ImVec2(center.x - size*0.7f, center.y), ImVec2(center.x + size*0.7f, center.y), col, thickness);
            ImGui::GetBackgroundDrawList()->AddLine(ImVec2(center.x, center.y - size*0.7f), ImVec2(center.x, center.y + size*0.7f), col, thickness);
            break;
        }
        case 3:
        {
            ImGui::GetBackgroundDrawList()->AddCircleFilled(center, size * 0.3f, col);
            break;
        }
        case 4:
        {
            float s = size * 0.7f;
            ImVec2 pts[4] = {
                ImVec2(center.x - s, center.y - s),
                ImVec2(center.x + s, center.y + s),
                ImVec2(center.x + s, center.y - s),
                ImVec2(center.x - s, center.y + s)
            };
            for (int i = 0; i < 4; i++) pts[i] = rotatePoint(pts[i], rotRad);
            ImGui::GetBackgroundDrawList()->AddLine(pts[0], pts[1], col, thickness);
            ImGui::GetBackgroundDrawList()->AddLine(pts[2], pts[3], col, thickness);
            break;
        }
        case 5:
        {
            float armLen = size * 1.2f;
            float innerGap = gap * 0.5f;
            
            ImVec2 top1(center.x + innerGap, center.y - innerGap);
            ImVec2 top2(center.x + innerGap, center.y - armLen);
            ImVec2 top3(center.x + armLen, center.y - armLen);
            
            ImVec2 right1(center.x + innerGap, center.y + innerGap);
            ImVec2 right2(center.x + armLen, center.y + innerGap);
            ImVec2 right3(center.x + armLen, center.y + armLen);
            
            ImVec2 bottom1(center.x - innerGap, center.y + innerGap);
            ImVec2 bottom2(center.x - innerGap, center.y + armLen);
            ImVec2 bottom3(center.x - armLen, center.y + armLen);
            
            ImVec2 left1(center.x - innerGap, center.y - innerGap);
            ImVec2 left2(center.x - armLen, center.y - innerGap);
            ImVec2 left3(center.x - armLen, center.y - armLen);
            
            top1 = rotatePoint(top1, rotRad); top2 = rotatePoint(top2, rotRad); top3 = rotatePoint(top3, rotRad);
            right1 = rotatePoint(right1, rotRad); right2 = rotatePoint(right2, rotRad); right3 = rotatePoint(right3, rotRad);
            bottom1 = rotatePoint(bottom1, rotRad); bottom2 = rotatePoint(bottom2, rotRad); bottom3 = rotatePoint(bottom3, rotRad);
            left1 = rotatePoint(left1, rotRad); left2 = rotatePoint(left2, rotRad); left3 = rotatePoint(left3, rotRad);
            
            ImGui::GetBackgroundDrawList()->AddLine(top1, top2, col, thickness * 1.5f);
            ImGui::GetBackgroundDrawList()->AddLine(top2, top3, col, thickness * 1.5f);
            
            ImGui::GetBackgroundDrawList()->AddLine(right1, right2, col, thickness * 1.5f);
            ImGui::GetBackgroundDrawList()->AddLine(right2, right3, col, thickness * 1.5f);
            
            ImGui::GetBackgroundDrawList()->AddLine(bottom1, bottom2, col, thickness * 1.5f);
            ImGui::GetBackgroundDrawList()->AddLine(bottom2, bottom3, col, thickness * 1.5f);
            
            ImGui::GetBackgroundDrawList()->AddLine(left1, left2, col, thickness * 1.5f);
            ImGui::GetBackgroundDrawList()->AddLine(left2, left3, col, thickness * 1.5f);
            break;
        }
        }

        if (sdk::crosshairDot)
        {
            ImGui::GetBackgroundDrawList()->AddCircleFilled(center, 2.0f, IM_COL32(255, 255, 255, 255));
        }
    }
}

#include "visuals.hpp"
#include "settings.hpp"
#include <deps/imgui/imgui.h>
#include <format>
#include <shellapi.h>

namespace visuals
{
    void RenderWatermark()
    {
        if (!sdk::watermark) return;

        float x = ImGui::GetIO().DisplaySize.x - 180.0f;
        float y = 10.0f;
        
        // Use menu colors
        ImU32 bgCol = IM_COL32(
            (int)(menu::bgColor[0] * 255),
            (int)(menu::bgColor[1] * 255),
            (int)(menu::bgColor[2] * 255),
            200
        );
        ImU32 accentCol = IM_COL32(
            (int)(menu::accentColor[0] * 255),
            (int)(menu::accentColor[1] * 255),
            (int)(menu::accentColor[2] * 255),
            255
        );
        ImU32 textCol = IM_COL32(255, 255, 255, 255);
        
        const char* text = "discord.gg/XRn3ZcXXNU";
        ImVec2 textSize = ImGui::CalcTextSize(text);
        float padding = 10.0f;
        float width = textSize.x + padding * 2;
        float height = textSize.y + padding * 1.5f;
        
        ImVec2 posMin(x, y);
        ImVec2 posMax(x + width, y + height);
        
        ImDrawList* draw = ImGui::GetBackgroundDrawList();
        
        ImVec2 mousePos = ImGui::GetIO().MousePos;
        bool isHovered = mousePos.x >= posMin.x && mousePos.x <= posMax.x && 
                         mousePos.y >= posMin.y && mousePos.y <= posMax.y;
        
        if (isHovered && ImGui::IsMouseClicked(0))
        {
            ShellExecuteA(NULL, "open", "https://discord.gg/XRn3ZcXXNU", NULL, NULL, SW_SHOWNORMAL);
        }
        
        if (isHovered)
        {
            bgCol = IM_COL32(
                (int)(menu::bgColor[0] * 255 + 30),
                (int)(menu::bgColor[1] * 255 + 30),
                (int)(menu::bgColor[2] * 255 + 30),
                220
            );
        }
        
        draw->AddRectFilled(posMin, posMax, bgCol, 8.0f);
        draw->AddRectFilled(ImVec2(posMin.x, posMin.y), ImVec2(posMax.x, posMin.y + 2), accentCol, 8.0f, ImDrawFlags_RoundCornersTop);
        draw->AddRectFilled(ImVec2(posMin.x, posMin.y + 2), ImVec2(posMax.x, posMin.y + 4), IM_COL32(255, 255, 255, 30), 6.0f, ImDrawFlags_RoundCornersTop);
        draw->AddRect(ImVec2(posMin.x + 1, posMin.y + 1), ImVec2(posMax.x + 1, posMax.y + 1), IM_COL32(0, 0, 0, 100), 8.0f);
        
        // Text
        ImVec2 textPos(x + padding, y + padding * 0.75f);
        draw->AddText(textPos, textCol, text);
    }

    void RenderFPS()
    {
        if (!sdk::fpsCounter) return;

        float fps = ImGui::GetIO().Framerate;
        std::string text = std::format("{:.0f} FPS", fps);
        
        // Use menu colors
        ImU32 bgCol = IM_COL32(
            (int)(menu::bgColor[0] * 255),
            (int)(menu::bgColor[1] * 255),
            (int)(menu::bgColor[2] * 255),
            200
        );
        ImU32 accentCol = IM_COL32(
            (int)(menu::accentColor[0] * 255),
            (int)(menu::accentColor[1] * 255),
            (int)(menu::accentColor[2] * 255),
            255
        );
        ImU32 fpsCol;
        if (fps > 60) {
            fpsCol = IM_COL32(0, 255, 100, 255);
        } else if (fps > 30) {
            fpsCol = IM_COL32(255, 200, 0, 255);
        } else {
            fpsCol = IM_COL32(255, 80, 80, 255);
        }
        
        ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
        float padding = 10.0f;
        float width = textSize.x + padding * 2;
        float height = textSize.y + padding * 1.5f;
        
        float x = 10.0f;
        float y = 10.0f;
        
        ImVec2 posMin(x, y);
        ImVec2 posMax(x + width, y + height);
        
        ImDrawList* draw = ImGui::GetBackgroundDrawList();
        
        draw->AddRectFilled(posMin, posMax, bgCol, 8.0f);
        draw->AddRectFilled(ImVec2(posMin.x, posMin.y), ImVec2(posMin.x + 3, posMax.y), accentCol, 8.0f, ImDrawFlags_RoundCornersLeft);
        draw->AddRectFilled(ImVec2(posMin.x + 3, posMin.y), ImVec2(posMax.x, posMin.y + 2), IM_COL32(255, 255, 255, 30), 6.0f, ImDrawFlags_RoundCornersTopRight);
        draw->AddRect(ImVec2(posMin.x + 1, posMin.y + 1), ImVec2(posMax.x + 1, posMax.y + 1), IM_COL32(0, 0, 0, 100), 8.0f);
        ImVec2 textPos(x + padding + 6, y + padding * 0.75f);
        draw->AddCircleFilled(ImVec2(posMin.x + padding * 0.5f + 3, posMin.y + height * 0.5f), 3.0f, fpsCol);
        draw->AddText(textPos, IM_COL32(255, 255, 255, 255), text.c_str());
    }
}

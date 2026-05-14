#define GLM_ENABLE_EXPERIMENTAL
#include <deps/glm/glm.hpp>
#include <deps/glm/gtc/type_ptr.hpp>
#include <deps/glm/gtx/string_cast.hpp>

#include "offsets.hpp"
#include "settings.hpp"
#include "config.hpp"
#include "menu.hpp"
#include "audio.hpp"
#include "../obfuscate_integration.hpp"

class c_overlay;
void RenderAim();
namespace config { void Load(); }
namespace menu { void Render(std::function<void()> overlayEnd); }
namespace ui { void LoadIntroFont(); bool ShowIntro(c_overlay& overlay); void ResetIntro(); }

#include <DirectXMath.h>
#include <Windows.h>
#include <deps/math/math.hpp>
#include <deps/memory/memory.h>
#include <deps/window/window.h>
#include <deps/imgui/imgui.h>
#include <d3d11.h>
#include <iostream>
#include <chrono>
#include <cmath>

ID3D11Device* g_pd3dDevice = nullptr;

extern std::unique_ptr<c_memory> memory;

namespace bloodstrike
{
    namespace renderer
    {
        uint64_t camera = 0;
        uint64_t localActor = 0;
    }
}

void update_camera_and_local();
glm::vec3 get_local_position();
bool w2s(uint64_t camera, const glm::vec3& worldPos, glm::vec2& screenPos, bool useAlternativeMethod);
void RenderAim();

int main()
{
    SetConsoleTitle(L"External Cheat Base");

    config::Load();

    JUNK();
    if (!memory->attach_to_process(L"BloodStrike.exe"))
    {
        JUNK();
        std::cin.get();
        return false;
    }

    offsets::auto_update_or_fallback();

    c_overlay overlay;
    if (!overlay.create())
    {
        std::cin.get();
        return false;
    }

    g_pd3dDevice = overlay.get_device();

    overlay.set_click_through(true);
    overlay.set_click_through(!menu::show);

    if (!sdk::disableIntro)
    {
        ui::LoadIntroFont();

        audio::PlayIntroMusic();

        bool showingIntro = true;
        ui::ResetIntro();
        while (overlay.running && showingIntro)
        {
            overlay.start();
            showingIntro = ui::ShowIntro(overlay);
            overlay.end();
        }
        menu::introComplete = true;
    }
    else
    {
        menu::introComplete = true;
    }

    auto startTime = std::chrono::steady_clock::now();
    bool showNotification = sdk::disableIntro;

    while (overlay.running)
    {
        if (GetAsyncKeyState(menu::toggleKey) & 1)
        {
            menu::show = !menu::show;
            overlay.set_click_through(!menu::show);
        }

        overlay.start();

        if (showNotification)
        {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - startTime).count();
            const float animDuration = 600.0f;
            const float holdDuration = 2000.0f;
            const float totalDuration = animDuration * 2 + holdDuration;

            if (elapsed < totalDuration)
            {
                ImDrawList* draw = ImGui::GetBackgroundDrawList();
                ImVec2 screenSize = ImGui::GetIO().DisplaySize;

                const float boxWidth = 350.0f;
                const float boxHeight = 60.0f;
                const float margin = 20.0f;
                const float targetX = screenSize.x - boxWidth - margin;

                float progress = 0.0f;
                float currentX = screenSize.x + 50;

                if (elapsed < animDuration)
                {
                    progress = elapsed / animDuration;
                    progress = 1.0f - std::pow(1.0f - progress, 3);
                    currentX = screenSize.x + 50 - (screenSize.x + 50 - targetX) * progress;
                }
                else if (elapsed < animDuration + holdDuration)
                {
                    currentX = targetX;
                    progress = 1.0f;
                }
                else
                {
                    float exitProgress = (elapsed - animDuration - holdDuration) / animDuration;
                    exitProgress = std::pow(exitProgress, 3);
                    currentX = targetX + (screenSize.x + 100 - targetX) * exitProgress;
                }

                float y = screenSize.y - 100;
                ImU32 bgColor = IM_COL32(
                    (int)(menu::bgColor[0] * 255),
                    (int)(menu::bgColor[1] * 255),
                    (int)(menu::bgColor[2] * 255),
                    (int)(menu::bgColor[3] * 230)
                );
                ImU32 accentColor = IM_COL32(
                    (int)(menu::accentColor[0] * 255),
                    (int)(menu::accentColor[1] * 255),
                    (int)(menu::accentColor[2] * 255),
                    255
                );

                draw->AddRectFilled(
                    ImVec2(currentX, y),
                    ImVec2(currentX + boxWidth, y + boxHeight),
                    bgColor, 12.0f
                );

                draw->AddRectFilled(
                    ImVec2(currentX, y + 10),
                    ImVec2(currentX + 4, y + boxHeight - 10),
                    accentColor, 2.0f
                );

                const char* msg = "INTRO DISABLED";
                const char* subMsg = "Loading directly...";

                draw->AddText(
                    ImVec2(currentX + 20, y + 12),
                    IM_COL32(255, 255, 255, 255),
                    msg
                );

                draw->AddText(
                    ImVec2(currentX + 20, y + 35),
                    IM_COL32(180, 180, 180, 255),
                    subMsg
                );

                float lineWidth = boxWidth * progress;
                draw->AddRectFilled(
                    ImVec2(currentX, y + boxHeight - 3),
                    ImVec2(currentX + lineWidth, y + boxHeight),
                    accentColor, 1.5f
                );
            }
            else
            {
                showNotification = false;
            }
        }

        RenderAim();

        if (menu::show)
        {
            menu::Render([&]() { overlay.end(); });
        }

        overlay.end();
    }

    return 0;
}

#include "menu.hpp"
#include "settings.hpp"
#include "config.hpp"
#include "triggerbot.hpp"
#include <deps/imgui/imgui.h>
#include <deps/imgui/imgui_internal.h>
#include <Windows.h>
#include <map>

extern std::map<int, float> menu::g_tab_anim;

static int active_tab = 0;
static int previous_tab = 0;
static float content_alpha = 1.0f;
static float content_slide = 0.0f;
static char keyName[32] = "RMB";
static bool waitingForKey = false;
static char menuKeyName[32] = "INSERT";
static bool waitingForMenuKey = false;
static float esp_preview_anim = 0.0f;
static bool show_esp_preview = false;

void menu::Render(std::function<void()> overlayEnd)
{
    ImGui::SetNextWindowSize({580, 480}, ImGuiCond_Once);
    ImGui::SetNextWindowPos({100, 100}, ImGuiCond_Once);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar;

    ImGui::SetNextWindowSizeConstraints(ImVec2(400, 300), ImVec2(1920, 1080));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImVec4 menuBg(menu::bgColor[0], menu::bgColor[1], menu::bgColor[2], menu::bgColor[3] * menu::alpha);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, menuBg);

    ImGui::Begin("BloodStrike EXTERNAL", &menu::show, window_flags);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();

    int alpha = (int)(menu::alpha * 255);
    ImU32 accentCol = IM_COL32((int)(menu::accentColor[0] * 255), (int)(menu::accentColor[1] * 255), (int)(menu::accentColor[2] * 255), (int)(menu::accentColor[3] * alpha));
    float titleHeight = 50.0f;

    ui::DrawSnowBackground(draw_list, windowSize.x, windowSize.y, menu::alpha, windowPos);

    ImU32 titleBgCol = IM_COL32((int)(menu::titleBg[0] * 255), (int)(menu::titleBg[1] * 255), (int)(menu::titleBg[2] * 255), (int)(menu::titleBg[3] * alpha));
    draw_list->AddRectFilled(windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y + titleHeight), titleBgCol, 16.0f, 0);

    ImVec2 titlePos = ImVec2(windowPos.x + 20, windowPos.y + 15);
    draw_list->AddText(titlePos, IM_COL32(255, 255, 255, 255), "BloodStrike");
    draw_list->AddText(ImVec2(titlePos.x + ImGui::CalcTextSize("BloodStrike").x + 8, titlePos.y), accentCol, "EXTERNAL");

    const char* tabs[] = {"Aimbot", "ESP", "Settings", "Crosshair", "Menu", "Config"};
    int tab_count = IM_ARRAYSIZE(tabs);

    float tabBarY = windowPos.y + titleHeight;
    float tabHeight = 45.0f;
    float tabWidth = windowSize.x / tab_count;

    ImU32 tabBgCol = IM_COL32((int)(menu::tabBg[0] * 255), (int)(menu::tabBg[1] * 255), (int)(menu::tabBg[2] * 255), (int)(menu::tabBg[3] * alpha));
    draw_list->AddRectFilled(ImVec2(windowPos.x, tabBarY), ImVec2(windowPos.x + windowSize.x, tabBarY + tabHeight), tabBgCol);

    for (int i = 0; i < tab_count; i++) {
        ImVec2 tabPos = ImVec2(windowPos.x + (i * tabWidth), tabBarY);
        bool isActive = (active_tab == i);
        float& anim = menu::g_tab_anim[i];
        float target = isActive ? 1.0f : 0.0f;
        anim += (target - anim) * ImGui::GetIO().DeltaTime * 12.0f;

        if (anim > 0.01f) {
            ImU32 tabBg = IM_COL32((int)(35 + (menu::accentColor[0] * 255 - 35) * anim), (int)(35 + (menu::accentColor[1] * 255 - 35) * anim), (int)(45 + (menu::accentColor[2] * 255 - 45) * anim), (int)(40 + (120 - 40) * anim));
            draw_list->AddRectFilled(ImVec2(tabPos.x + 4, tabPos.y + 6), ImVec2(tabPos.x + tabWidth - 4, tabPos.y + tabHeight - 6), tabBg, 8.0f);
            
            float scale = 1.0f + (anim * 0.05f);
            ImVec2 center(tabPos.x + tabWidth / 2, tabPos.y + tabHeight / 2);
            float glowSize = anim * 20.0f;
            if (glowSize > 0.5f) {
                draw_list->AddRectFilled(ImVec2(center.x - glowSize, tabBarY + tabHeight - 3), ImVec2(center.x + glowSize, tabBarY + tabHeight - 1), accentCol, 2.0f);
            }
            if (isActive) {
                float indicatorWidth = tabWidth * (0.3f + anim * 0.4f);
                draw_list->AddRectFilled(ImVec2(tabPos.x + (tabWidth - indicatorWidth) / 2, tabPos.y + tabHeight - 4), ImVec2(tabPos.x + (tabWidth + indicatorWidth) / 2, tabPos.y + tabHeight - 2), accentCol, 2.0f);
            }
        }

        ImGui::SetCursorScreenPos(ImVec2(tabPos.x + 2, tabPos.y + 8));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.25f, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.35f, 0.5f));
        
        float textAlpha = 0.5f + anim * 0.5f;
        float textScale = 1.0f + anim * 0.08f;
        ImVec4 textCol = isActive ? ImVec4(1, 1, 1, 1) : ImVec4(0.5f, 0.5f, 0.55f, textAlpha);
        ImGui::PushStyleColor(ImGuiCol_Text, textCol);
        
        ImVec2 btnSize = ImVec2(tabWidth - 4, tabHeight - 14);
        if (ImGui::Button(tabs[i], btnSize)) {
            if (active_tab != i) {
                previous_tab = active_tab;
                active_tab = i;
                content_alpha = 0.0f;
                content_slide = (i > previous_tab) ? 30.0f : -30.0f;
                show_esp_preview = (i == 1);
            }
        }
        ImGui::PopStyleColor(4);
    }

    float target_alpha = 1.0f;
    float target_slide = 0.0f;
    content_alpha += (target_alpha - content_alpha) * ImGui::GetIO().DeltaTime * 10.0f;
    content_slide += (target_slide - content_slide) * ImGui::GetIO().DeltaTime * 12.0f;

    float contentY = tabBarY + tabHeight + 10;
    float contentHeight = windowSize.y - tabHeight - titleHeight - 20;

    ImGui::SetCursorScreenPos(ImVec2(windowPos.x + 20 + content_slide * (1.0f - content_alpha), contentY));
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, content_alpha);
    ImGui::BeginChild("Content", ImVec2(windowSize.x - 40, contentHeight), false);
    ImGui::Indent(8.0f);

    switch (active_tab) {
    case 0: {
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.96f, 1), "Aimbot");
        ImGui::Spacing();
        ui::CustomToggle("Enable Aimbot", &sdk::aimbotEnabled);
        ImGui::Spacing();
        ui::CustomToggle("Draw FOV Circle", &sdk::aimbotDrawFov);
        ImGui::Spacing();
        ui::CustomToggle("Team Check", &sdk::aimbotTeamCheck);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ui::CustomSliderFloat("FOV Range", &sdk::aimbotFov, 5.0f, 800.0f, "%.0f");
        ImGui::Spacing();
        ui::CustomSliderFloat("Smoothness", &sdk::aimbotSmooth, 0.0f, 20.0f, "%.1f");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Keybind");
        if (waitingForKey) {
            ui::CustomButton("Press any key...", ImVec2(150, 32));
            for (int vk = 0x01; vk < 0xFF; vk++) {
                if (vk == VK_INSERT) continue;
                if (GetAsyncKeyState(vk) & 0x8000) {
                    sdk::aimbotKey = vk;
                    waitingForKey = false;
                    switch(vk) {
                    case VK_LBUTTON: strcpy_s(keyName, "LMB"); break;
                    case VK_RBUTTON: strcpy_s(keyName, "RMB"); break;
                    case VK_MBUTTON: strcpy_s(keyName, "MMB"); break;
                    case VK_SHIFT: strcpy_s(keyName, "Shift"); break;
                    case VK_CONTROL: strcpy_s(keyName, "Ctrl"); break;
                    case VK_MENU: strcpy_s(keyName, "Alt"); break;
                    case VK_SPACE: strcpy_s(keyName, "Space"); break;
                    default:
                        if (vk >= 'A' && vk <= 'Z') sprintf_s(keyName, "%c", vk);
                        else if (vk >= VK_F1 && vk <= VK_F12) sprintf_s(keyName, "F%d", vk - VK_F1 + 1);
                        break;
                    }
                    break;
                }
            }
        } else {
            char btnLabel[64];
            sprintf_s(btnLabel, "%s", keyName);
            if (ui::CustomButton(btnLabel, ImVec2(100, 32))) waitingForKey = true;
        }
        ImGui::Spacing();
        ImGui::Text("Target Bone");
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.08f, 0.11f, 1));
        ImGui::Combo("##bone", &sdk::aimbotTargetBone, sdk::boneNames, sdk::boneNameCount);
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.96f, 1), "Triggerbot");
        ImGui::Spacing();
        ui::CustomToggle("Enable Triggerbot", &triggerbot::enabled);
        ImGui::Spacing();
        ui::CustomSliderFloat("Trigger Delay (ms)", &triggerbot::delayMs, 0.0f, 200.0f, "%.0f");
        ImGui::Spacing();
        ui::CustomSliderFloat("Max Distance", &triggerbot::maxDistance, 10.0f, 200.0f, "%.0f");
        ImGui::Spacing();
        ui::CustomSliderFloat("Head Only", &triggerbot::headOnly, 0.0f, 1.0f, "%.1f");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.96f, 1), "FOV Circle Colors");
        ImGui::Spacing();
        ImGui::Text("FOV Inactive");
        ImGui::ColorEdit4("##fovinactive", colors::fovInactive, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoLabel);
        ImGui::Spacing();
        ImGui::Text("FOV and crosshair Active");
        ImGui::ColorEdit4("##fovactive", colors::fovActive, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoLabel);
        break;
    }
    case 1: {
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.96f, 1), "ESP");
        ImGui::Spacing();
        ui::CustomToggle("Skeleton", &sdk::espSkeleton);
        ImGui::Spacing();
        ui::CustomToggle("Box", &sdk::espBox);
        ImGui::Spacing();
        ImGui::Text("Box Style");
        const char* boxStyles[] = {"Corners", "Rectangle", "Rounded", "Filled", "3D Box"};
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.08f, 0.11f, 1));
        ImGui::Combo("##boxstyle", &sdk::espBoxStyle, boxStyles, IM_ARRAYSIZE(boxStyles));
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ui::CustomToggle("Distance", &sdk::espDistance);
        ImGui::Spacing();
        ui::CustomToggle("Tracers", &sdk::espTracers);
        ImGui::Spacing();
        ui::CustomToggle("Radar", &sdk::espRadar);
        ImGui::Spacing();
        if (sdk::espRadar) {
            ui::CustomSliderFloat("Radar Zoom", &sdk::espRadarZoom, 0.20f, 4.00f, "%.2fx");
            ImGui::Spacing();
            ui::CustomSliderFloat("Radar Size", &sdk::espRadarSize, 80.0f, 350.0f, "%.0f");
            ImGui::Spacing();
            ui::CustomSliderFloat("Radar Enemy Size", &sdk::espRadarEnemySize, 1.0f, 8.0f, "%.1f");
            ImGui::Spacing();
        }
        ui::CustomToggle("Chinese Hat", &sdk::espChapeauChinois);
        ImGui::Spacing();
        ui::CustomSliderFloat("Rotation Speed", &sdk::espChapeauRotationSpeed, 0.0f, 10.0f, "%.1f");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.96f, 1), "ESP Distance");
        ImGui::Spacing();
        ui::CustomSliderFloat("Max Distance", &sdk::espMaxDistance, 10.0f, 700.0f, "%.0fm");
        ImGui::SameLine();
        ImGui::Text("%.0fm", sdk::espMaxDistance);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.96f, 1), "ESP Colors");
        ImGui::Spacing();
        ImGui::Text("Box");
        ImGui::ColorEdit4("##box", colors::espBox, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoLabel);
        ImGui::Spacing();
        ImGui::Text("Distance");
        ImGui::ColorEdit4("##dist", colors::espDistance, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoLabel);
        ImGui::Spacing();
        ImGui::Text("Skeleton");
        ImGui::ColorEdit4("##bones", colors::espBones, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoLabel);
        ImGui::Spacing();
        ImGui::Text("Tracers");
        ImGui::ColorEdit4("##lines", colors::espLines, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoLabel);
        ImGui::Spacing();
        ImGui::Text("Chapeau Chinois");
        ImGui::ColorEdit4("##chapeau", colors::espChapeau, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoLabel);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.96f, 1), "Loot ESP");
        ImGui::Spacing();
        ui::CustomToggle("Enable Loot ESP", &sdk::espLoot);
        ImGui::Spacing();
        ui::CustomSliderFloat("Loot Distance", &sdk::espLootMaxDistance, 10.0f, 200.0f, "%.0fm");
        ImGui::Spacing();
        ImGui::Text("Loot Color");
        ImGui::ColorEdit4("##loot", colors::espLoot, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoLabel);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.96f, 1), "Bot Detection");
        ImGui::Spacing();
        ui::CustomToggle("Hide Bots", &sdk::espHideBots);
        ImGui::Spacing();
        ui::CustomToggle("Show Bots (different color)", &sdk::espShowBots);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.96f, 1), "Debug");
        ImGui::Spacing();
        ui::CustomToggle("Entity Scan Debug", &sdk::debugEntityScan);
        ImGui::Spacing();
        break;
    }
    case 2: {
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.96f, 1), "Settings");
        ImGui::Spacing();
        ui::CustomToggle("Watermark", &sdk::watermark);
        ImGui::Spacing();
        ui::CustomToggle("FPS Counter", &sdk::fpsCounter);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.96f, 1), "Startup");
        ImGui::Spacing();
        ui::CustomToggle("Disable Intro", &sdk::disableIntro);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.96f, 1), "Info");
        ImGui::Text("BloodStrike EXTERNAL");
        ImGui::Text("External Cheat");
        break;
    }
    case 3: {
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.96f, 1), "Crosshair");
        ImGui::Spacing();
        ui::CustomToggle("Enable Crosshair", &sdk::customCrosshair);
        ImGui::Spacing();
        ImGui::Text("Style");
        const char* styles[] = {"Cross", "Circle", "Cross+Circle", "Dot", "X", "Swastika"};
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.08f, 0.11f, 1));
        ImGui::Combo("##style", &sdk::crosshairStyle, styles, IM_ARRAYSIZE(styles));
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ui::CustomSliderFloat("Size", &sdk::crosshairSize, 2.0f, 30.0f, "%.0f");
        ImGui::Spacing();
        ui::CustomSliderFloat("Thickness", &sdk::crosshairThickness, 1.0f, 5.0f, "%.1f");
        ImGui::Spacing();
        ui::CustomSliderFloat("Gap", &sdk::crosshairGap, 0.0f, 10.0f, "%.0f");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.96f, 1), "Rotation");
        ImGui::Spacing();
        ui::CustomSliderFloat("Rotation Speed", &sdk::crosshairRotationSpeed, 0.0f, 10.0f, "%.1f");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.96f, 1), "Crosshair Color");
        ImGui::Spacing();
        ImGui::ColorEdit4("##crosshaircolor", sdk::crosshairColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoLabel);
        ImGui::Spacing();
        ui::CustomToggle("Center Dot", &sdk::crosshairDot);
        break;
    }
    case 4: {
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.96f, 1), "Menu");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.96f, 1), "Menu Settings");
        ImGui::Spacing();
        ImGui::Text("Menu Toggle Key");
        if (waitingForMenuKey) {
            ui::CustomButton("Press any key...", ImVec2(150, 32));
            for (int vk = 0x01; vk < 0xFF; vk++) {
                if (GetAsyncKeyState(vk) & 0x8000) {
                    menu::toggleKey = vk;
                    waitingForMenuKey = false;
                    switch(vk) {
                    case VK_LBUTTON: strcpy_s(menuKeyName, "LMB"); break;
                    case VK_RBUTTON: strcpy_s(menuKeyName, "RMB"); break;
                    case VK_MBUTTON: strcpy_s(menuKeyName, "MMB"); break;
                    case VK_SHIFT: strcpy_s(menuKeyName, "Shift"); break;
                    case VK_CONTROL: strcpy_s(menuKeyName, "Ctrl"); break;
                    case VK_MENU: strcpy_s(menuKeyName, "Alt"); break;
                    case VK_SPACE: strcpy_s(menuKeyName, "Space"); break;
                    case VK_INSERT: strcpy_s(menuKeyName, "Insert"); break;
                    case VK_DELETE: strcpy_s(menuKeyName, "Delete"); break;
                    case VK_HOME: strcpy_s(menuKeyName, "Home"); break;
                    case VK_END: strcpy_s(menuKeyName, "End"); break;
                    case VK_PRIOR: strcpy_s(menuKeyName, "PgUp"); break;
                    case VK_NEXT: strcpy_s(menuKeyName, "PgDn"); break;
                    case VK_TAB: strcpy_s(menuKeyName, "Tab"); break;
                    case VK_CAPITAL: strcpy_s(menuKeyName, "Caps"); break;
                    case VK_ESCAPE: strcpy_s(menuKeyName, "Esc"); break;
                    default:
                        if (vk >= 'A' && vk <= 'Z') sprintf_s(menuKeyName, "%c", vk);
                        else if (vk >= '0' && vk <= '9') sprintf_s(menuKeyName, "%c", vk);
                        else if (vk >= VK_F1 && vk <= VK_F12) sprintf_s(menuKeyName, "F%d", vk - VK_F1 + 1);
                        else sprintf_s(menuKeyName, "Key %d", vk);
                        break;
                    }
                    break;
                }
            }
        } else {
            char btnLabel[64];
            sprintf_s(btnLabel, "%s", menuKeyName);
            if (ui::CustomButton(btnLabel, ImVec2(100, 32))) waitingForMenuKey = true;
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ui::CustomSliderFloat("Menu Transparency", &menu::alpha, 0.5f, 1.0f, "%.2f");
        ImGui::Spacing();
        ImGui::Text("Background Effect");
        const char* effects[] = {"None", "Snow", "Rain", "Floating Particles", "Stars", "Bubbles", "Matrix", "Fireflies", "Grid", "Waves", "Confetti", "Fire", "Plasma", "Nebula", "Shooting Stars"};
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.08f, 0.11f, 1));
        ImGui::Combo("##effect", &menu::backgroundEffect, effects, IM_ARRAYSIZE(effects));
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ui::CustomSliderFloat("Effect Intensity", &menu::effectIntensity, 10.0f, 200.0f, "%.0f");
        ImGui::Spacing();
        if (menu::backgroundEffect != 0) {
            ImGui::Text("Effect Color");
            ImGui::ColorEdit4("##effectcolor", menu::effectColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoLabel);
            ImGui::Spacing();
        }
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.96f, 1), "Menu Colors");
        ImGui::Spacing();
        ImGui::Text("Background");
        ImGui::ColorEdit4("##menubg", menu::bgColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoLabel);
        ImGui::Spacing();
        ImGui::Text("Accent");
        ImGui::ColorEdit4("##menuaccent", menu::accentColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoLabel);
        ImGui::Spacing();
        ImGui::Text("Title Bar");
        ImGui::ColorEdit4("##menutitle", menu::titleBg, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoLabel);
        ImGui::Spacing();
        ImGui::Text("Tab Bar");
        ImGui::ColorEdit4("##menutab", menu::tabBg, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoLabel);
        break;
    }
    case 5: {
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.96f, 1), "Config");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.96f, 1), "Configuration");
        ImGui::Spacing();
        ImGui::Text("Save and load your settings");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        if (ui::CustomButton("Save Config", ImVec2(120, 32))) config::Save();
        ImGui::SameLine();
        if (ui::CustomButton("Load Config", ImVec2(120, 32))) config::Load();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.56f, 0.27f, 0.96f, 1), "Config File");
        ImGui::Text("Settings are saved to config.ini");
        break;
    }
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    
    ImVec2 mainPos = ImGui::GetWindowPos();
    ImVec2 mainSize = ImGui::GetWindowSize();
    
    ImGui::End();
    
    if (active_tab == 1) {
        float previewWidth = 220.0f;
        
        ImVec2 previewPos(
            mainPos.x + mainSize.x - 2.0f,
            mainPos.y
        );
        
        ImGui::SetNextWindowPos(previewPos);
        ImGui::SetNextWindowSize(ImVec2(previewWidth, mainSize.y));
        
        ImGuiWindowFlags previewFlags = ImGuiWindowFlags_NoCollapse | 
                                       ImGuiWindowFlags_NoResize | 
                                       ImGuiWindowFlags_NoMove |
                                       ImGuiWindowFlags_NoTitleBar |
                                       ImGuiWindowFlags_NoScrollbar |
                                       ImGuiWindowFlags_NoBringToFrontOnFocus;
        
        ImVec4 previewBg = ImVec4(0.10f, 0.10f, 0.14f, 0.98f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, previewBg);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 16.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        
        if (ImGui::Begin("ESP Preview", nullptr, previewFlags)) {
            ImDrawList* draw = ImGui::GetWindowDrawList();
            ImVec2 winPos = ImGui::GetWindowPos();
            ImVec2 winSize = ImGui::GetWindowSize();
            float centerX = winPos.x + winSize.x * 0.5f;
            float centerY = winPos.y + winSize.y * 0.5f;
            
            ImTextureID enemyTex = sdk::espSkeleton ? ui::GetSkeletonImageTexture() : ui::GetEnemyImageTexture();
            if (enemyTex) {
                float imgSize = 220.0f;
                ImVec2 imgPos(
                    winPos.x + (winSize.x - imgSize) * 0.5f,
                    centerY - imgSize * 0.5f - 20.0f
                );
                draw->AddImage(
                    enemyTex,
                    imgPos,
                    ImVec2(imgPos.x + imgSize, imgPos.y + imgSize),
                    ImVec2(0, 0), ImVec2(1, 1),
                    IM_COL32(255, 255, 255, 200)
                );
            }
            
            float boxW = 130.0f;
            float boxH = 240.0f;
            float boxX = centerX - boxW * 0.5f;
            float boxY = centerY - boxH * 0.5f - 20.0f;
            
            if (sdk::espBox) {
                ImU32 boxColor = IM_COL32(
                    (int)(colors::espBox[0] * 255),
                    (int)(colors::espBox[1] * 255),
                    (int)(colors::espBox[2] * 255),
                    (int)(colors::espBox[3] * 255)
                );
                
                switch (sdk::espBoxStyle) {
                case 0: {
                    float cs = 25.0f;
                    float t = 1.5f;
                    draw->AddLine(ImVec2(boxX, boxY), ImVec2(boxX + cs, boxY), boxColor, t);
                    draw->AddLine(ImVec2(boxX, boxY), ImVec2(boxX, boxY + cs), boxColor, t);
                    draw->AddLine(ImVec2(boxX + boxW, boxY), ImVec2(boxX + boxW - cs, boxY), boxColor, t);
                    draw->AddLine(ImVec2(boxX + boxW, boxY), ImVec2(boxX + boxW, boxY + cs), boxColor, t);
                    draw->AddLine(ImVec2(boxX, boxY + boxH), ImVec2(boxX + cs, boxY + boxH), boxColor, t);
                    draw->AddLine(ImVec2(boxX, boxY + boxH), ImVec2(boxX, boxY + boxH - cs), boxColor, t);
                    draw->AddLine(ImVec2(boxX + boxW, boxY + boxH), ImVec2(boxX + boxW - cs, boxY + boxH), boxColor, t);
                    draw->AddLine(ImVec2(boxX + boxW, boxY + boxH), ImVec2(boxX + boxW, boxY + boxH - cs), boxColor, t);
                    break;
                }
                case 1:
                    draw->AddRect(ImVec2(boxX, boxY), ImVec2(boxX + boxW, boxY + boxH), boxColor, 0.0f, 0, 1.5f);
                    break;
                case 2:
                    draw->AddRect(ImVec2(boxX, boxY), ImVec2(boxX + boxW, boxY + boxH), boxColor, 8.0f, 0, 1.5f);
                    break;
                case 3: {
                    ImU32 fill = IM_COL32(
                        (int)(colors::espBox[0] * 255),
                        (int)(colors::espBox[1] * 255),
                        (int)(colors::espBox[2] * 255),
                        64
                    );
                    draw->AddRectFilled(ImVec2(boxX, boxY), ImVec2(boxX + boxW, boxY + boxH), fill, 4.0f);
                    draw->AddRect(ImVec2(boxX, boxY), ImVec2(boxX + boxW, boxY + boxH), boxColor, 4.0f, 0, 1.5f);
                    break;
                }
                case 4: {
                    float offset = boxW * 0.15f;
                    
                    ImVec2 backTL(boxX + offset, boxY - offset * 0.5f);
                    ImVec2 backTR(boxX + boxW + offset, boxY - offset * 0.5f);
                    ImVec2 backBL(boxX + offset, boxY + boxH - offset * 0.5f);
                    ImVec2 backBR(boxX + boxW + offset, boxY + boxH - offset * 0.5f);
                    
                    ImU32 backFill = IM_COL32(
                        (int)(colors::espBox[0] * 255 * 0.6f),
                        (int)(colors::espBox[1] * 255 * 0.6f),
                        (int)(colors::espBox[2] * 255 * 0.6f),
                        128
                    );
                    
                    draw->AddQuadFilled(backTL, backTR, backBR, backBL, backFill);
                    
                    draw->AddLine(backTL, ImVec2(boxX, boxY), boxColor, 1.5f);
                    draw->AddLine(backTR, ImVec2(boxX + boxW, boxY), boxColor, 1.5f);
                    draw->AddLine(backBL, ImVec2(boxX, boxY + boxH), boxColor, 1.5f);
                    draw->AddLine(backBR, ImVec2(boxX + boxW, boxY + boxH), boxColor, 1.5f);
                    
                    draw->AddQuad(backTL, backTR, backBR, backBL, boxColor, 1.5f);
                    
                    draw->AddRect(ImVec2(boxX, boxY), ImVec2(boxX + boxW, boxY + boxH), boxColor, 0.0f, 0, 2.0f);
                    break;
                }
                }
            }
            
            if (sdk::espDistance) {
                ImU32 distColor = IM_COL32(
                    (int)(colors::espDistance[0] * 255),
                    (int)(colors::espDistance[1] * 255),
                    (int)(colors::espDistance[2] * 255),
                    (int)(colors::espDistance[3] * 255)
                );
                const char* distText = "25m";
                ImVec2 ts = ImGui::CalcTextSize(distText);
                draw->AddText(ImVec2(centerX - ts.x * 0.5f, boxY + boxH + 5.0f), distColor, distText);
            }
            
            if (sdk::espTracers) {
                ImU32 lineColor = IM_COL32(
                    (int)(colors::espLines[0] * 255),
                    (int)(colors::espLines[1] * 255),
                    (int)(colors::espLines[2] * 255),
                    (int)(colors::espLines[3] * 255)
                );
                float sby = winPos.y + winSize.y - 30.0f;
                draw->AddLine(ImVec2(centerX, sby), ImVec2(centerX, boxY + boxH), lineColor, 1.5f);
            }
            
            if (sdk::espChapeauChinois) {
                ImU32 chapeauColor = IM_COL32(
                    (int)(colors::espChapeau[0] * 255),
                    (int)(colors::espChapeau[1] * 255),
                    (int)(colors::espChapeau[2] * 255),
                    (int)(colors::espChapeau[3] * 255)
                );
                
                float chapeauSize = 45.0f;
                float chapeauHeight = 35.0f;
                float chapeauX = centerX;
                float chapeauY = boxY + 25.0f;
                
                ImVec2 base1(chapeauX - chapeauSize, chapeauY);
                ImVec2 base2(chapeauX, chapeauY + chapeauSize * 0.3f);
                ImVec2 base3(chapeauX + chapeauSize, chapeauY);
                ImVec2 base4(chapeauX, chapeauY - chapeauSize * 0.3f);
                
                ImVec2 top(chapeauX, chapeauY - chapeauHeight);
                
                ImU32 fillColor = IM_COL32(
                    (int)(colors::espChapeau[0] * 255),
                    (int)(colors::espChapeau[1] * 255),
                    (int)(colors::espChapeau[2] * 255),
                    64
                );
                
                draw->AddTriangleFilled(base1, base2, top, fillColor);
                draw->AddTriangleFilled(base2, base3, top, fillColor);
                draw->AddTriangleFilled(base3, base4, top, fillColor);
                draw->AddTriangleFilled(base4, base1, top, fillColor);
                
                draw->AddLine(base1, base2, chapeauColor, 1.5f);
                draw->AddLine(base2, base3, chapeauColor, 1.5f);
                draw->AddLine(base3, base4, chapeauColor, 1.5f);
                draw->AddLine(base4, base1, chapeauColor, 1.5f);
                draw->AddLine(base1, top, chapeauColor, 1.5f);
                draw->AddLine(base2, top, chapeauColor, 1.5f);
                draw->AddLine(base3, top, chapeauColor, 1.5f);
                draw->AddLine(base4, top, chapeauColor, 1.5f);
            }
        }
        ImGui::End();
        
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
    }
}

#pragma once
#include <functional>
#include <map>
#include <deps/imgui/imgui.h>
#include <memory>
#include <deps/memory/memory.h>

class c_overlay;

namespace ui
{
    bool CustomToggle(const char* label, bool* v, float width = 50.0f, float height = 26.0f);
    bool CustomSliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.1f");
    bool CustomButton(const char* label, const ImVec2& size_arg = ImVec2(0, 0));

    void DrawSnowBackground(ImDrawList* draw_list, float windowWidth, float windowHeight, float menuAlpha, ImVec2 windowPos);
    void LoadIntroFont();
    bool ShowIntro(c_overlay& overlay);
    void ResetIntro();
    ImTextureID GetEnemyImageTexture();
    void FreeEnemyImageTexture();
    ImTextureID GetSkeletonImageTexture();
}

namespace menu
{
    extern std::map<int, float> g_tab_anim;
    void Render(std::function<void()> overlayEnd);
}

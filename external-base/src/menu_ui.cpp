#include "menu.hpp"
#include "settings.hpp"
#include "config.hpp"
#include "font_embedded.hpp"
#include "image_embedded.hpp"
#include "skeleton_embedded.hpp"
#include <deps/imgui/imgui.h>
#include <deps/imgui/imgui_internal.h>
#include <Windows.h>
#include <vector>
#include <map>
#include <chrono>
#include <cstdio>

#define STB_IMAGE_IMPLEMENTATION
#include <deps/stb_image.h>

#include <d3d11.h>

static std::map<ImGuiID, float> g_toggle_anim;
static std::map<ImGuiID, float> g_slider_anim;
static std::map<ImGuiID, float> g_button_anim;

namespace menu {
    std::map<int, float> g_tab_anim;
}

struct EffectParticle {
    float x, y;
    float speed;
    float size;
    float alpha;
    float dx;
    float dy;
};
static std::vector<EffectParticle> effectParticles;
static int lastEffectType = -1;

struct IntroParticle {
    float x, y;
    float vx, vy;
    float size;
    float alpha;
    int sides;
};
static std::vector<IntroParticle> introParticles;
static bool introInitialized = false;
static ImFont* introFont = nullptr;

void ui::LoadIntroFont() {
    ImGuiIO& io = ImGui::GetIO();
    introFont = io.Fonts->AddFontFromMemoryTTF(
        (void*)gfs_didot_font_data, 
        gfs_didot_font_size, 
        120.0f
    );
    if (!introFont) {
        introFont = io.Fonts->AddFontDefault();
    }
    io.Fonts->Build();
}

struct AnimationHelper {
    static float Lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }
};

static void InitIntroParticles(float width, float height) {
    introParticles.clear();
    int count = 80;
    for (int i = 0; i < count; i++) {
        IntroParticle p;
        p.x = (float)(rand() % (int)width);
        p.y = (float)(rand() % (int)height);
        float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
        float speed = 20.0f + (float)(rand() % 40);
        p.vx = cosf(angle) * speed;
        p.vy = sinf(angle) * speed;
        p.size = 4.0f + (float)(rand() % 8);
        p.alpha = 0.4f + (float)(rand() % 60) / 100.0f;
        p.sides = 3 + (rand() % 4);
        introParticles.push_back(p);
    }
    introInitialized = true;
}

static void UpdateIntroParticles(float width, float height, float dt) {
    for (auto& p : introParticles) {
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        
        if (p.x < 0 || p.x > width) p.vx *= -1;
        if (p.y < 0 || p.y > height) p.vy *= -1;
        
        p.x = ImClamp(p.x, 0.0f, width);
        p.y = ImClamp(p.y, 0.0f, height);
    }
}

static void DrawPolygon(ImDrawList* draw, ImVec2 center, float radius, int sides, ImU32 col, float rotation = 0) {
    if (sides < 3) sides = 3;
    ImVec2 points[6];
    for (int i = 0; i < sides && i < 6; i++) {
        float angle = rotation + (float)i * 2.0f * 3.14159f / (float)sides;
        points[i] = ImVec2(center.x + cosf(angle) * radius, center.y + sinf(angle) * radius);
    }
    if (sides == 3) {
        draw->AddTriangleFilled(points[0], points[1], points[2], col);
        draw->AddTriangle(points[0], points[1], points[2], col, 1.0f);
    } else if (sides == 4) {
        draw->AddRectFilled(points[0], points[2], col, 0, 0);
        draw->AddRect(points[0], points[2], col, 0, 0, 1.0f);
    } else {
        draw->AddConvexPolyFilled(points, sides, col);
        draw->AddPolyline(points, sides, col, true, 1.0f);
    }
}

bool ui::ShowIntro(c_overlay& overlay) {
    static auto startTime = std::chrono::steady_clock::now();
    static float introAlpha = 1.0f;
    
    auto now = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration<float>(now - startTime).count();
    
    if (elapsed > 4.0f) return false;
    
    if (elapsed > 3.5f) {
        introAlpha = 1.0f - (elapsed - 3.5f) * 2.0f;
        if (introAlpha < 0) introAlpha = 0;
    }
    
    ImVec2 screenSize = ImGui::GetIO().DisplaySize;
    float width = screenSize.x;
    float height = screenSize.y;
    
    if (!introInitialized) {
        InitIntroParticles(width, height);
    }
    
    UpdateIntroParticles(width, height, ImGui::GetIO().DeltaTime);
    
    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    
    draw->AddRectFilled(ImVec2(0, 0), screenSize, IM_COL32(10, 10, 14, (int)(255 * introAlpha)));
    
    float connectDist = 120.0f;
    ImU32 lineCol = IM_COL32(
        (int)(menu::accentColor[0] * 255),
        (int)(menu::accentColor[1] * 255),
        (int)(menu::accentColor[2] * 255),
        (int)(100 * introAlpha)
    );
    
    for (size_t i = 0; i < introParticles.size(); i++) {
        for (size_t j = i + 1; j < introParticles.size(); j++) {
            float dx = introParticles[j].x - introParticles[i].x;
            float dy = introParticles[j].y - introParticles[i].y;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist < connectDist) {
                float alpha = (1.0f - dist / connectDist) * introAlpha;
                ImU32 col = IM_COL32(
                    (int)(menu::accentColor[0] * 255),
                    (int)(menu::accentColor[1] * 255),
                    (int)(menu::accentColor[2] * 255),
                    (int)(alpha * 100)
                );
                draw->AddLine(
                    ImVec2(introParticles[i].x, introParticles[i].y),
                    ImVec2(introParticles[j].x, introParticles[j].y),
                    col,
                    1.0f
                );
            }
        }
    }
    
    for (auto& p : introParticles) {
        ImU32 col = IM_COL32(
            (int)(menu::accentColor[0] * 255),
            (int)(menu::accentColor[1] * 255),
            (int)(menu::accentColor[2] * 255),
            (int)(p.alpha * 255 * introAlpha)
        );
        DrawPolygon(draw, ImVec2(p.x, p.y), p.size, p.sides, col, elapsed * 0.5f);
    }
    
    if (introFont) {
        ImGui::PushFont(introFont);
        const char* text = "DEKO";
        ImVec2 textSize = ImGui::CalcTextSize(text);
        ImVec2 textPos(width * 0.5f - textSize.x * 0.5f, height * 0.5f - textSize.y * 0.5f);
        
        ImU32 shadowCol = IM_COL32(0, 0, 0, (int)(150 * introAlpha));
        draw->AddText(ImVec2(textPos.x + 4, textPos.y + 4), shadowCol, text);
        
        ImU32 textCol = IM_COL32(255, 255, 255, (int)(255 * introAlpha));
        draw->AddText(textPos, textCol, text);
        ImGui::PopFont();
    }
    
    return true;
}

// Image texture globals
static ImTextureID g_enemyImageTexture = 0;
static int g_enemyImageWidth = 0;
static int g_enemyImageHeight = 0;
static ImTextureID g_skeletonImageTexture = 0;

extern ID3D11Device* g_pd3dDevice;

ImTextureID ui::GetEnemyImageTexture() {
    if (g_enemyImageTexture) {
        return g_enemyImageTexture;
    }
    
    if (!g_pd3dDevice) {
        return 0;
    }
    
    int width, height, channels;
    unsigned char* data = stbi_load_from_memory(
        enemy_png_data, 
        enemy_png_size, 
        &width, 
        &height, 
        &channels, 
        4
    );
    
    if (!data) {
        return 0;
    }
    
    g_enemyImageWidth = width;
    g_enemyImageHeight = height;
    
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    
    D3D11_SUBRESOURCE_DATA subResource = {};
    subResource.pSysMem = data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    
    ID3D11Texture2D* pTexture = nullptr;
    HRESULT hr = g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);
    
    if (FAILED(hr)) {
        stbi_image_free(data);
        return 0;
    }
    
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    
    ID3D11ShaderResourceView* pSRV = nullptr;
    hr = g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &pSRV);
    
    pTexture->Release();
    stbi_image_free(data);
    
    if (FAILED(hr)) {
        return 0;
    }
    
    g_enemyImageTexture = (ImTextureID)pSRV;
    return g_enemyImageTexture;
}

ImTextureID ui::GetSkeletonImageTexture() {
    if (g_skeletonImageTexture) {
        return g_skeletonImageTexture;
    }
    
    if (!g_pd3dDevice) {
        return 0;
    }
    
    int width, height, channels;
    unsigned char* data = stbi_load_from_memory(
        skeleton_png_data, 
        skeleton_png_size, 
        &width, 
        &height, 
        &channels, 
        4
    );
    
    if (!data) {
        return 0;
    }
    
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    
    D3D11_SUBRESOURCE_DATA subResource = {};
    subResource.pSysMem = data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    
    ID3D11Texture2D* pTexture = nullptr;
    HRESULT hr = g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);
    
    if (FAILED(hr)) {
        stbi_image_free(data);
        return 0;
    }
    
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    
    ID3D11ShaderResourceView* pSRV = nullptr;
    hr = g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &pSRV);
    
    pTexture->Release();
    stbi_image_free(data);
    
    if (FAILED(hr)) {
        return 0;
    }
    
    g_skeletonImageTexture = (ImTextureID)pSRV;
    return g_skeletonImageTexture;
}

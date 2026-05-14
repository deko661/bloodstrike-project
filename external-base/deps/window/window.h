#pragma once
#include <windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <chrono>
#include <thread>
#include <dwmapi.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx11.h"
#include "../imgui/imgui_internal.h"
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")

using Microsoft::WRL::ComPtr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

class c_overlay
{
public:
    c_overlay(HWND target_window = nullptr);
    ~c_overlay();

    bool running;
    bool create();
    void destroy();
    void start();
    void end();
    void set_click_through(bool enable);
    HWND window() const
    {
        return m_hWnd;
    }

    ID3D11Device* get_device() const
    {
        return m_pd3dDevice.Get();
    }

    float desired_fps = 240;

private:
    bool init_window();
    bool init_d3d();
    void cleanup_d3d();
    void create_render_target();
    void cleanup_render_target();

    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

    HWND m_hWnd;
    HINSTANCE m_hInst;

    ComPtr<ID3D11Device> m_pd3dDevice;
    ComPtr<ID3D11DeviceContext> m_pd3dDeviceContext;
    ComPtr<IDXGISwapChain> m_pSwapChain;
    ComPtr<ID3D11RenderTargetView> m_mainRenderTargetView;

    int current_width;
    int current_height;
    std::chrono::high_resolution_clock::time_point next_frame_time;
};

#include "window.h"

static const wchar_t CLASS_NAME[] = L"ovl_fullscreen";

c_overlay::c_overlay(HWND target_window)
    : m_hWnd(nullptr), m_hInst(GetModuleHandle(nullptr)), current_width(0), current_height(0)
{
    running = false;
    next_frame_time = std::chrono::high_resolution_clock::now();
}

c_overlay::~c_overlay()
{
    destroy();
}

bool c_overlay::create()
{
    if (!init_window())
        return false;
    if (!init_d3d())
        return false;

    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;

    // Load modern fonts
    ImGuiIO& io = ImGui::GetIO();
    
    // Primary font - Inter or Segoe UI with larger size
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    
    // Bold font for headers
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeuib.ttf", 20.0f);
    
    // Small font for labels
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 14.0f);
    
    io.Fonts->Build();

    ImGui_ImplWin32_Init(m_hWnd);
    ImGui_ImplDX11_Init(m_pd3dDevice.Get(), m_pd3dDeviceContext.Get());

    ImGuiStyle &st = ImGui::GetStyle();
    
    // Enable anti-aliasing for smooth rendering
    st.AntiAliasedLines = true;
    st.AntiAliasedFill = true;
    st.AntiAliasedLinesUseTex = true;
    st.CurveTessellationTol = 1.25f;
    st.CircleTessellationMaxError = 0.30f;

    // Premium rounded styling
    st.WindowRounding = 16.0f;
    st.ChildRounding = 12.0f;
    st.FrameRounding = 8.0f;
    st.PopupRounding = 12.0f;
    st.ScrollbarRounding = 10.0f;
    st.GrabRounding = 12.0f;
    st.TabRounding = 8.0f;
    
    // Remove borders for clean look
    st.WindowBorderSize = 0.0f;
    st.ChildBorderSize = 0.0f;
    st.PopupBorderSize = 0.0f;
    st.FrameBorderSize = 0.0f;
    
    // Better spacing
    st.ScrollbarSize = 10.0f;
    st.GrabMinSize = 16.0f;
    st.WindowPadding = ImVec2(16, 16);
    st.FramePadding = ImVec2(12, 8);
    st.ItemSpacing = ImVec2(12, 10);
    st.ItemInnerSpacing = ImVec2(8, 6);
    st.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    st.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    st.SelectableTextAlign = ImVec2(0.0f, 0.5f);

    // Modern Dark Theme with Accent
    auto &c = st.Colors;
    
    // Premium accent - Electric Purple to Blue gradient feel
    ImVec4 accentPrimary = ImVec4(0.56f, 0.27f, 0.96f, 1.00f);    // #8F4AF6
    ImVec4 accentHover = ImVec4(0.64f, 0.40f, 1.00f, 1.00f);      // #A366FF
    ImVec4 accentActive = ImVec4(0.45f, 0.20f, 0.85f, 1.00f);    // #7333D9
    ImVec4 accentGlow = ImVec4(0.56f, 0.27f, 0.96f, 0.40f);     // Glow effect
    
    // Background palette - Deep dark with subtle blue tint
    ImVec4 bgPrimary = ImVec4(0.06f, 0.06f, 0.08f, 0.98f);      // Main bg
    ImVec4 bgSecondary = ImVec4(0.09f, 0.09f, 0.12f, 0.95f);    // Child windows
    ImVec4 bgTertiary = ImVec4(0.12f, 0.12f, 0.16f, 0.90f);     // Elevated surfaces
    ImVec4 bgInput = ImVec4(0.08f, 0.08f, 0.11f, 1.00f);       // Input fields
    
    // Window
    c[ImGuiCol_WindowBg] = bgPrimary;
    c[ImGuiCol_ChildBg] = bgSecondary;
    c[ImGuiCol_PopupBg] = bgSecondary;
    
    // Title bar
    c[ImGuiCol_TitleBg] = ImVec4(0.05f, 0.05f, 0.07f, 1.00f);
    c[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.06f, 0.12f, 1.00f);
    c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.05f, 0.05f, 0.07f, 1.00f);
    
    // Text
    c[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.98f, 1.00f);
    c[ImGuiCol_TextDisabled] = ImVec4(0.45f, 0.45f, 0.50f, 1.00f);
    c[ImGuiCol_TextSelectedBg] = ImVec4(accentPrimary.x, accentPrimary.y, accentPrimary.z, 0.30f);
    
    // Borders (subtle)
    c[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.25f, 0.40f);
    c[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    
    // Separators
    c[ImGuiCol_Separator] = ImVec4(0.18f, 0.18f, 0.22f, 0.60f);
    c[ImGuiCol_SeparatorHovered] = accentGlow;
    c[ImGuiCol_SeparatorActive] = accentPrimary;
    
    // Frames (inputs, checkboxes)
    c[ImGuiCol_FrameBg] = bgInput;
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.12f, 0.16f, 1.00f);
    c[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);
    
    // Buttons
    c[ImGuiCol_Button] = bgTertiary;
    c[ImGuiCol_ButtonHovered] = accentHover;
    c[ImGuiCol_ButtonActive] = accentActive;
    
    // Headers (collapsing headers, tree nodes)
    c[ImGuiCol_Header] = bgTertiary;
    c[ImGuiCol_HeaderHovered] = accentGlow;
    c[ImGuiCol_HeaderActive] = accentPrimary;
    
    // Tabs
    c[ImGuiCol_Tab] = bgInput;
    c[ImGuiCol_TabHovered] = accentGlow;
    c[ImGuiCol_TabActive] = accentPrimary;
    c[ImGuiCol_TabUnfocused] = bgInput;
    c[ImGuiCol_TabUnfocusedActive] = ImVec4(accentPrimary.x, accentPrimary.y, accentPrimary.z, 0.70f);
    
    // Sliders and checkboxes
    c[ImGuiCol_SliderGrab] = accentPrimary;
    c[ImGuiCol_SliderGrabActive] = accentHover;
    c[ImGuiCol_CheckMark] = accentPrimary;
    
    // Scrollbars
    c[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.07f, 0.00f);
    c[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.25f, 0.30f, 0.80f);
    c[ImGuiCol_ScrollbarGrabHovered] = accentHover;
    c[ImGuiCol_ScrollbarGrabActive] = accentActive;
    
    // Misc
    c[ImGuiCol_ResizeGrip] = accentGlow;
    c[ImGuiCol_ResizeGripHovered] = accentHover;
    c[ImGuiCol_ResizeGripActive] = accentActive;
    c[ImGuiCol_DragDropTarget] = accentGlow;
    c[ImGuiCol_NavHighlight] = accentPrimary;
    c[ImGuiCol_NavWindowingHighlight] = accentGlow;
    c[ImGuiCol_NavWindowingDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
    
    // Plot lines/histogram
    c[ImGuiCol_PlotLines] = accentPrimary;
    c[ImGuiCol_PlotLinesHovered] = accentHover;
    c[ImGuiCol_PlotHistogram] = accentPrimary;
    c[ImGuiCol_PlotHistogramHovered] = accentHover;
    
    // Table headers
    c[ImGuiCol_TableHeaderBg] = bgTertiary;
    c[ImGuiCol_TableBorderStrong] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);
    c[ImGuiCol_TableBorderLight] = ImVec4(0.20f, 0.20f, 0.25f, 0.50f);
    c[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    c[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.03f);

    running = true;
    return true;
}

void c_overlay::destroy()
{
    if (!m_hWnd)
        return;

    running = false;

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    cleanup_d3d();
    DestroyWindow(m_hWnd);
    UnregisterClassW(CLASS_NAME, m_hInst);

    m_hWnd = nullptr;
}

void c_overlay::start()
{
    auto interval = std::chrono::duration<double>(1.0 / desired_fps);
    auto now = std::chrono::high_resolution_clock::now();
    // Only sleep if we're running faster than desired (uncapped if behind)
    if (next_frame_time > now)
        std::this_thread::sleep_until(next_frame_time);
    // Reset timing from now to prevent accumulation lag
    next_frame_time = now + std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(interval);

    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
        if (msg.message == WM_QUIT)
            running = false;
    }

    if (!running)
        return;

    RECT rc;
    GetClientRect(m_hWnd, &rc);
    UINT w = rc.right;
    UINT h = rc.bottom;

    if ((int)w != current_width || (int)h != current_height)
    {
        current_width = w;
        current_height = h;
        ImGui_ImplDX11_InvalidateDeviceObjects();
        cleanup_render_target();
        m_pSwapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
        create_render_target();
        ImGui_ImplDX11_CreateDeviceObjects();
    }

    const float clr[4] = {0, 0, 0, 0};
    m_pd3dDeviceContext->OMSetRenderTargets(1, m_mainRenderTargetView.GetAddressOf(), nullptr);
    m_pd3dDeviceContext->ClearRenderTargetView(m_mainRenderTargetView.Get(), clr);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void c_overlay::end()
{
    if (!running)
        return;
    ImGui::Render();
    m_pd3dDeviceContext->OMSetRenderTargets(1, m_mainRenderTargetView.GetAddressOf(), nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    m_pSwapChain->Present(0, 0);
}

void c_overlay::set_click_through(bool enable)
{
    LONG ex = GetWindowLongW(m_hWnd, GWL_EXSTYLE);

    if (enable)
        ex |= WS_EX_TRANSPARENT | WS_EX_NOACTIVATE;
    else
    {
        ex &= ~WS_EX_TRANSPARENT;
        ex &= ~WS_EX_NOACTIVATE;
        SetForegroundWindow(m_hWnd);
    }

    SetWindowLongW(m_hWnd, GWL_EXSTYLE, ex);
    SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
}

bool c_overlay::init_window()
{
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = m_hInst;
    wc.lpszClassName = CLASS_NAME;
    RegisterClassExW(&wc);

    RECT r;
    GetClientRect(GetDesktopWindow(), &r);

    m_hWnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
        CLASS_NAME,
        L"",
        WS_POPUP,
        0, 0,
        r.right, r.bottom,
        nullptr, nullptr, m_hInst, this);

    if (!m_hWnd)
        return false;

    MARGINS m = {-1};
    DwmExtendFrameIntoClientArea(m_hWnd, &m);
    SetLayeredWindowAttributes(m_hWnd, 0, 255, LWA_ALPHA);

    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);

    return true;
}

bool c_overlay::init_d3d()
{
    RECT rc;
    GetClientRect(m_hWnd, &rc);

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = rc.right;
    sd.BufferDesc.Height = rc.bottom;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;

    D3D_FEATURE_LEVEL fl;
    D3D_FEATURE_LEVEL fls[] = {D3D_FEATURE_LEVEL_11_0};

    if (FAILED(D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            fls, 1, D3D11_SDK_VERSION,
            &sd, &m_pSwapChain, &m_pd3dDevice,
            &fl, &m_pd3dDeviceContext)))
        return false;

    create_render_target();
    return true;
}

void c_overlay::cleanup_d3d()
{
    cleanup_render_target();
    if (m_pd3dDeviceContext)
        m_pd3dDeviceContext->ClearState();
    m_pSwapChain.Reset();
    m_pd3dDeviceContext.Reset();
    m_pd3dDevice.Reset();
}

void c_overlay::create_render_target()
{
    ComPtr<ID3D11Texture2D> bb;
    m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&bb));
    m_pd3dDevice->CreateRenderTargetView(bb.Get(), nullptr, &m_mainRenderTargetView);
}

void c_overlay::cleanup_render_target()
{
    m_mainRenderTargetView.Reset();
}

LRESULT CALLBACK c_overlay::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    c_overlay *o = (c_overlay *)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

    if (msg == WM_CREATE)
    {
        auto cs = (CREATESTRUCTW *)lParam;
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
        return 0;
    }

    if (o)
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

    if (msg == WM_DESTROY)
    {
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}
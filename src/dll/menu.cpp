#include "menu.h"
#include "common.h"
#include "settings.h"

#include <filesystem>
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <atlbase.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Settings g_settings;

IDXGISwapChain*         g_swapChain = nullptr;
ID3D11DeviceContext*    g_context = nullptr;
ID3D11RenderTargetView* g_renderTargetView = nullptr;
HWND                    g_hWindow = nullptr;
WNDPROC                 g_originalWndProcHandler = nullptr;

bool g_menuInitialised = false;
//bool g_showDemoWindow = false;
bool g_showMenu = false;
#ifdef ALIASISOLATION_ENABLE_PROFILER
std::string g_profilerStats = "";
extern bool g_shaderHooksEnabled;
#endif // ALIASISOLATION_ENABLE_PROFILER

// Constructor for the Menu class.
Menu::Menu() {
}

// Destructor for the Menu class.
Menu::~Menu() {
    g_swapChain = nullptr;
    g_context = nullptr;
    g_renderTargetView = nullptr;
    g_hWindow = nullptr;
    g_originalWndProcHandler = nullptr;
}

// Our custom window handler to override A:I's own handler.
// This lets us detect keyboard inputs and pass mouse + keyboard input control to ImGui.
LRESULT CALLBACK Menu::WndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    ImGuiIO& io = ImGui::GetIO();
    POINT cursorPos;
    GetCursorPos(&cursorPos);
    ScreenToClient(g_hWindow, &cursorPos);
    io.MousePos.x = cursorPos.x;
    io.MousePos.y = cursorPos.y;

    if (uMsg == WM_KEYUP) {
        if (wParam == VK_DELETE) {
            g_showMenu = !g_showMenu;
        }
    }

    if (g_showMenu) {
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
        return true;
    }

    return CallWindowProc(g_originalWndProcHandler, hWnd, uMsg, wParam, lParam);
}

void Menu::InitMenu(IDXGISwapChain* pSwapChain) {
    g_swapChain = pSwapChain;

    // We were loading cinematic tools far too early, and it couldn't access the D3D swap chain in the engine.
    // Now, we inject it only after knowing that the engine has at least created its device and swap chain.
    if (std::filesystem::exists("cinematicTools.dll") && GetModuleHandleA("AI.exe"))
    {
      LoadLibraryA("cinematicTools.dll");
      LOG_MSG("[aliasIsolation::dllMain] LoadLibraryA(\"cinematicTools.dll\")\n", "");
    }
}

void Menu::DrawMenu() {
    if (!g_menuInitialised) {
        if (FAILED(GetDeviceAndContextFromSwapChain(g_swapChain, &g_device, &g_context))) {
            DebugBreak();
            return;
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        // Get the swapchain description (we use this to get the handle to the game's window).
        DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
        DX_CHECK(g_swapChain->GetDesc(&dxgiSwapChainDesc));

        ImGui::StyleColorsDark();

        g_hWindow = dxgiSwapChainDesc.OutputWindow;
        // Replace the game's window handler with our own so we can intercept key events in the game.
        g_originalWndProcHandler = reinterpret_cast<WNDPROC>(SetWindowLongPtr(g_hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Menu::WndProcHandler)));

        ImGui_ImplWin32_Init(g_hWindow);
        ImGui_ImplDX11_Init(g_device, g_context);

        ID3D11Texture2D* pBackBuffer = nullptr;
        DX_CHECK(g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pBackBuffer)));

        // Ensure the back buffer is not 0 or null.
        if (pBackBuffer) {
            DX_CHECK(g_device->CreateRenderTargetView(pBackBuffer, NULL, &g_renderTargetView));
        }
        else {
            DebugBreak();
        }

        pBackBuffer->Release();

        g_menuInitialised = true;
    }
    
    // Feed inputs to dear imgui, start new frame.
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

#ifdef ALIASISOLATION_ENABLE_PROFILER
    {
      ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
      ImGui::Begin("Alias Isolation - Profiler", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
#ifdef ALIASISOLATION_NO_SMAA_JITTER_ADD
      ImGui::TextColored(ImVec4(255, 0, 0, 1), "No SMAA jitter calc.");
#endif
#ifdef ALIASISOLATION_NO_JITTER_REMOVE
      ImGui::TextColored(ImVec4(255, 0, 0, 1), "No jitter removal");
#endif
#ifdef ALIASISOLATION_NO_JITTER_ADD
      ImGui::TextColored(ImVec4(255, 0, 0, 1), "No jitter calculation");
#endif
#ifdef ALIASISOLATION_NO_RGBM_VS
      ImGui::TextColored(ImVec4(255, 0, 0, 1), "No RGBM vertex shader");
#endif
#ifdef ALIASISOLATION_NO_SMAA_VS
      ImGui::TextColored(ImVec4(255, 0, 0, 1), "No SMAA vertex shader");
#endif
#ifdef ALIASISOLATION_NO_VS_OVERRIDES
      ImGui::TextColored(ImVec4(255, 0, 0, 1), "No vertex shader overrides");
#endif
#ifdef ALIASISOLATION_NO_PS_OVERRIDES
      ImGui::TextColored(ImVec4(255, 0, 0, 1), "No pixel shader overrides");
#endif
#ifdef ALIASISOLATION_NO_TAA_PASS
      ImGui::TextColored(ImVec4(255, 0, 0, 1), "No TAA pass");
#endif
#ifdef ALIASISOLATION_NO_CA_PASS
      ImGui::TextColored(ImVec4(255, 0, 0, 1), "No CA pass");
#endif
      if (!g_shaderHooksEnabled) {
        ImGui::TextColored(ImVec4(255, 0, 0, 1), "Hooks disabled");
      }
      else {
        ImGui::TextColored(ImVec4(255, 170, 0, 1), g_profilerStats.c_str());
      }
      ImGui::End();
    }
#endif // ALIASISOLATION_ENABLE_PROFILER

    // If the user wants to display the menu, then render it.
    if (g_showMenu) {
        static bool aliasIsolation_menu_showAboutWindow = false;
        
        if (aliasIsolation_menu_showAboutWindow) {
            if (ImGui::Begin("Alias Isolation - About", &aliasIsolation_menu_showAboutWindow, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
                    ImGui::TextColored(ImVec4(255, 170, 0, 1), "Alias Isolation 1.1.3 - Built on %s at %s", __DATE__, __TIME__);
                ImGui::Text("Dear ImGui %s", ImGui::GetVersion());
                ImGui::Separator();
                ImGui::Text("Build information:");
#ifdef _DEBUG
                ImGui::Text("Target: Debug");
#else
                ImGui::Text("Target: Release");
#endif
#ifdef _WIN32
                ImGui::Text("Architecture: x86");
#elif _WIN64
                ImGui::Text("Architecture: x64");
#endif
#ifdef _MSC_VER
                ImGui::Text("MSVC Compiler Version: %d (%d)", _MSC_VER, _MSC_FULL_VER);
#endif
#ifdef _MSVC_LANG
                ImGui::Text("MSVC C++ Standard: %d", _MSVC_LANG);
#endif
                ImGui::Separator();
                ImGui::Text("Mod credits:");
                ImGui::Text("Alias Isolation originally by aliasIsolation.");
                ImGui::Text("Currently maintained by RyanJGray.");
                ImGui::Text("Thanks for making this mod a reality, aliasIsolation - whoever you are!");
            }

            ImGui::End();
        }

        // Get the center of the screen.
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        {
                ImGui::Begin("Alias Isolation - Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoSavedSettings);

            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("Help")) {
                    if (ImGui::MenuItem("About")) {
                        aliasIsolation_menu_showAboutWindow = true;
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }

                ImGui::TextColored(ImVec4(255, 170, 0, 1), "Alias Isolation 1.1.3");

            ImGui::Separator();

                ImGui::Checkbox("Sharpening", &g_settings.sharpeningEnabled);
                if (g_settings.sharpeningEnabled) {
                    ImGui::SliderFloat("Sharpening Amount", &g_settings.sharpening, 0.0f, 1.0f, "%.2f");
                }

                ImGui::Checkbox("Chromatic Aberration", &g_settings.chromaticAberrationEnabled);
                if (g_settings.chromaticAberrationEnabled) {
                    ImGui::SliderFloat("Chromatic Aberration Amount", &g_settings.chromaticAberration, 0.0f, 1.0f, "%.2f");
                }

            if (ImGui::Button("Save Settings")) {
              saveSettings(g_settings);
            }

            ImGui::End();
        }
    }

    ImGui::EndFrame();

    // Render dear imgui onto the screen.
    ImGui::Render();
    g_context->OMSetRenderTargets(1, &g_renderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // A call to DXGISwapChain->Present is not needed here, as we are hooked into DirectX 11's Present function, so anything we draw here will be presented by the game.
}

void Menu::ShutdownMenu() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    g_menuInitialised = false;
}

// Code taken (and changed a little) from Niemand's wonderful article on hooking DX11 and using ImGUI.
// https://niemand.com.ar/2019/01/01/how-to-hook-directx-11-imgui/
HRESULT Menu::GetDeviceAndContextFromSwapChain(IDXGISwapChain* pSwapChain, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext) {
    HRESULT ret = pSwapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<PVOID*>(ppDevice));

    if (SUCCEEDED(ret)) {
        (*ppDevice)->GetImmediateContext(ppContext);
    }

    return ret;
}
#pragma once

#include <d3d11.h>

static bool g_aliasIsolation_mod_enabled = true;

class Menu
{
public:
	Menu();
	~Menu();
	static LRESULT CALLBACK Menu::WndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void InitMenu(IDXGISwapChain* pSwapChain);
	static void DrawMenu();
	static void ShutdownMenu();
	static HRESULT GetDeviceAndContextFromSwapChain(IDXGISwapChain* pSwapChain, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext);
};
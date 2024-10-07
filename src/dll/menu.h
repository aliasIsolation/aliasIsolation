#pragma once

#include <d3d11.h>

class Menu
{
public:
	static LRESULT CALLBACK WndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void InitMenu(IDXGISwapChain& swapChain);
	static void DrawMenu();
    static void CreateRenderTarget();
    static void CleanupRenderTarget();
	static void ShutdownMenu();
};
#pragma once

#include <d3d11.h>
#include <atlbase.h>

class Menu
{
public:
	static LRESULT CALLBACK WndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void InitMenu(const CComPtr<IDXGISwapChain>& swapChain);
	static void DrawMenu();
	static void HandleResize(const CComPtr<IDXGISwapChain>& swapChain);
	static void ShutdownMenu();
};
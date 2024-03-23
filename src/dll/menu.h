#pragma once

#include <d3d11.h>

class Menu
{
public:
	static LRESULT CALLBACK WndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void InitMenu(IDXGISwapChain* pSwapChain);
	static void DrawMenu();
	static void ShutdownMenu();
	static HRESULT GetDeviceAndContextFromSwapChain(IDXGISwapChain* pSwapChain, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext);
};
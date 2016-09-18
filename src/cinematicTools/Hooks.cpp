#include "Hooks.h"
#include <Windows.h>
#include <memory>
#include "Tools\Log.h"
#include "MinHook.h"
#include "Offsets.h"
#include "Main.h"
#include <d3d11.h>
#include "FW1FontWrapper.h"
#include "WICTextureLoader.h"
#include "SpriteBatch.h"
//#include "resource.h"

#define PRESENT_OFFSET 0xFAAD
#define REDUCEHEALTH 0x475280

typedef int(__thiscall* tPostProcess)(void*);
typedef unsigned __int8(__thiscall* tCamera)(void*);
typedef char(__thiscall* tInput)(void*);
typedef int(__thiscall* tRotation)(void*, void*, void*, void*, void*);
typedef char(__thiscall* tPlayerHook)(void*, int);
typedef int(__stdcall* tAppProfile)(int a1, int a2, int a3);
typedef int(__thiscall* tHelmet)(void*, int);
typedef void(__thiscall* tCharacter)(void*);
typedef char(__thiscall* tFreeze1)(void*, signed int);
typedef int(__thiscall* tCamera2)(void*, int, int);
typedef float(__thiscall* tCamera3)(void*, int, int, int);
typedef void(__thiscall* tUI)(void*, int);

tPostProcess oPostProcess = NULL;
tCamera oCamera = NULL;
tHelmet oHelmet = NULL;
tInput oInput = NULL;
tRotation oRotation = NULL;
tPlayerHook oPlayerHook = NULL;
tCharacter oCharacter = NULL;
tFreeze1 oFreeze1 = NULL;
tCamera2 oCamera2 = NULL;
tCamera3 oCamera3 = NULL;
tUI oUI = NULL;

tAppProfile oAppProfile = NULL;

SpriteBatch* pSpriteBatch;
ID3D11Resource* resource;
ID3D11ShaderResourceView* resourceView;

bool que = false;
ID3D11Device *pDevice = NULL;
ID3D11DeviceContext *pContext = NULL;
IFW1Factory *pFW1Factory = NULL;
IFW1FontWrapper *pFontWrapper = NULL;

void InitSpriteBatch()
{
	pSpriteBatch = new SpriteBatch(pContext);
}

void drawSpriteBatch()
{
	pSpriteBatch->Begin();
	pSpriteBatch->Draw(resourceView, Main::position);
	pSpriteBatch->End();
}

int __stdcall hAppProfile(int a1, int a2, int a3)
{
	if (!que)
	{
		que = true;
		IDXGISwapChain* pSwapChain = (IDXGISwapChain*)a1;
		pSwapChain->GetDevice(__uuidof(pDevice), (void**)&pDevice);
		pDevice->GetImmediateContext(&pContext);

		IFW1Factory* pFW1Factory;
		FW1CreateFactory(FW1_VERSION, &pFW1Factory);
		pFW1Factory->CreateFontWrapper(pDevice, L"Tahoma", &pFontWrapper);
		pFW1Factory->Release();

	}
	else 
		pFontWrapper->DrawString(pContext, L"It works", 50.0f, 100.0f, 100.0f, 0xFFFFFFFF, 0);
	return oAppProfile(a1, a2, a3);
}

/*
bool firstTime;
bool FW1_Successful;

DWORD* pSwapChainVtable = NULL;
DWORD* pDeviceContextVTable = NULL;

ID3D11Device *pDevice = NULL;
ID3D11DeviceContext *pContext = NULL;
IDXGISwapChain* pSwapChain = NULL;
IFW1FontWrapper *pFontWrapper = NULL;

void* phookD3D11Present = NULL;
void* detourBuffer = NULL;



*/

/*
void __declspec(naked) PresentNakedHookFunc()
{
	__asm
	{
		push esi;
	}

	if (firstTime)
	{
		__asm
		{
			push eax
				mov eax, [ebp + 8]
				mov pSwapChain, eax
				pop eax
		}

		pSwapChain->GetDevice(__uuidof(pDevice), (void**)&pDevice);
		pDevice->GetImmediateContext(&pContext);

		IFW1Factory* pFW1Factory;
		FW1CreateFactory(FW1_VERSION, &pFW1Factory);
		pFW1Factory->CreateFontWrapper(pDevice, L"Tahoma", &pFontWrapper);
		pFW1Factory->Release();

		HMODULE dwModule = GetModuleHandle("AlienIsolation.dll");

		printf("dwModule: %X\n", dwModule);

		HRSRC hRes = FindResource(dwModule, MAKEINTRESOURCE(IDR_0_1), "0");
		printf("hRes: %X\n", hRes);
		HGLOBAL hData = LoadResource(dwModule, hRes);
		printf("hData: %X\n", hData);
		printf("Error: %X\n", GetLastError());
		LPVOID data = LockResource(hData);
		printf("Data: %X\n", data);
		DWORD size = SizeofResource(dwModule, hRes);
		printf("Size: %X\n", size);

		Sleep(100);

		HRESULT result = NULL; //CreateWICTextureFromFile(pDevice, L"test.jpg", &resource, &resourceView);
		DWORD dwVirtualProtectBackup;
		VirtualProtect(data, size, PAGE_READWRITE, &dwVirtualProtectBackup);
		result = CreateWICTextureFromMemory(pDevice, (const uint8_t*)data, (size_t)size, &resource, &resourceView);
		//result = CreateWICTextureFromMemoryEx(pDevice, pContext, (const uint8_t*)data, (size_t)size, 0, D3D11_USAGE_DEFAULT, 0, 0, 0, false, &resource, &resourceView);
		if (FAILED(result))
			printf("Failed\n");

		printf("Error: %X\n", GetLastError());

		InitSpriteBatch();

		//Main::m_menu.Init(pSwapChain);

		firstTime = false;
	}

	pFontWrapper->DrawString(pContext, L"It works", 50.0f, 100.0f, 100.0f, 0xFFFFFFFF, 0);
	//Main::m_menu.Draw();
	//drawSpriteBatch();

	__asm
	{
		pop esi
			jmp phookD3D11Present
	}
}

const void* __cdecl DetourFunc(BYTE* src, const BYTE* dest, const DWORD length)
{
	BYTE* jump = new BYTE[length + 5];
	detourBuffer = jump;

	DWORD dwVirtualProtectBackup;
	VirtualProtect(src, length, PAGE_READWRITE, &dwVirtualProtectBackup);

	memcpy(jump, src, length);
	jump += length;

	jump[0] = 0xE9;
	*(DWORD*)(jump + 1) = (DWORD)(src + length - jump) - 5;

	src[0] = 0xE9;
	*(DWORD*)(src + 1) = (DWORD)(dest - src) - 5;

	VirtualProtect(src, length, dwVirtualProtectBackup, &dwVirtualProtectBackup);

	return jump - length;
}

void InitializeHook()
{
	HWND hWnd = FindWindow(NULL, "Alien: Isolation");
	DWORD dwVersion = GetVersion();
	int offset = NULL;

	DWORD dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
	DWORD dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));


	if (dwMinorVersion < 2)
		offset = 0x19;
	else
		offset = 0xB;

	Log::Write("Major: " + to_string(dwMajorVersion) + " Minor: " + to_string(dwMinorVersion));

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = GetWindowLong(hWnd, GWL_STYLE) & WS_POPUP != 0 ? FALSE : TRUE;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, &featureLevel, 1
		, D3D11_SDK_VERSION, &swapChainDesc, &pSwapChain, &pDevice, NULL, &pContext)))
	{
		Log::WriteError("Couldn't create device");
		return;
	}

	pSwapChainVtable = (DWORD*)pSwapChain;
	pSwapChainVtable = (DWORD*)pSwapChainVtable[0];

	pDeviceContextVTable = (DWORD*)pContext;
	pDeviceContextVTable = (DWORD*)pDeviceContextVTable[0];

	phookD3D11Present = (void*)DetourFunc((BYTE*)((int)pSwapChainVtable[8] + offset), (BYTE*)PresentNakedHookFunc, 5);

	DWORD dwOld;
	VirtualProtect(phookD3D11Present, 2, PAGE_EXECUTE_READWRITE, &dwOld);

	pDevice->Release();
	pContext->Release();
	pSwapChain->Release();

	return;
}
*/

typedef HRESULT(WINAPI * tD3D11Present)(IDXGISwapChain*, UINT, UINT);
typedef void(__stdcall *D3D11DrawIndexedHook) (ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
typedef void(__stdcall *D3D11ClearRenderTargetViewHook) (ID3D11DeviceContext* pContext, ID3D11RenderTargetView *pRenderTargetView, const FLOAT ColorRGBA[4]);

DWORD* pSwapChainVtable = NULL;
DWORD* pDeviceContextVTable = NULL;

tD3D11Present phookD3D11Present = NULL;
D3D11DrawIndexedHook phookD3D11DrawIndexed = NULL;
D3D11ClearRenderTargetViewHook phookD3D11ClearRenderTargetView = NULL;

bool firstTime = true;
void* detourBuffer = NULL;

HRESULT WINAPI hD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (firstTime)
	{
		firstTime = false;
		Main::m_menu.Init(pSwapChain);
	}
	
	Main::m_menu.Draw();

	return phookD3D11Present(pSwapChain, SyncInterval, Flags);
}

void __cdecl overlayRenderHook(ID3D11Device* device, ID3D11DeviceContext* context)
{
	if (firstTime)
	{
		firstTime = false;
		Main::m_menu.Init(device, context);
	}

	Main::m_menu.Draw();
}

void __stdcall hookD3D11DrawIndexed(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
	return phookD3D11DrawIndexed(pContext, IndexCount, StartIndexLocation, BaseVertexLocation);
}

void __stdcall hookD3D11ClearRenderTargetView(ID3D11DeviceContext* pContext, ID3D11RenderTargetView *pRenderTargetView, const FLOAT ColorRGBA[4])
{
	return phookD3D11ClearRenderTargetView(pContext, pRenderTargetView, ColorRGBA);
}

const void* __cdecl DetourFunc(BYTE* src, const BYTE* dest, const DWORD length)
{
	BYTE* jump = new BYTE[length + 5];
	detourBuffer = jump;

	DWORD dwVirtualProtectBackup;
	VirtualProtect(src, length, PAGE_READWRITE, &dwVirtualProtectBackup);

	memcpy(jump, src, length);
	jump += length;

	jump[0] = 0xE9;
	*(DWORD*)(jump + 1) = (DWORD)(src + length - jump) - 5;

	src[0] = 0xE9;
	*(DWORD*)(src + 1) = (DWORD)(dest - src) - 5;

	VirtualProtect(src, length, dwVirtualProtectBackup, &dwVirtualProtectBackup);

	return jump - length;
}

bool failed = false;

DWORD __stdcall InitializeHook(LPVOID)
{
	//Sleep(2000);
	HWND hWnd = GetForegroundWindow();
	if (!hWnd)
		Log::WriteError("Couldn't get AI.exe");
	IDXGISwapChain* pSwapChain;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = GetWindowLong(hWnd, GWL_STYLE) & WS_POPUP != 0 ? FALSE : TRUE;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	Sleep(100);

	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, &featureLevel, 1
		, D3D11_SDK_VERSION, &swapChainDesc, &pSwapChain, &pDevice, NULL, &pContext);

	if (FAILED(hr))
	{
		Log::WriteError("Couldn't create DX device");
		Log::WriteError("Error code: " + Log::convertToHexa(hr));
		failed = true;
		return NULL;
	}

	pSwapChainVtable = (DWORD*)pSwapChain;
	pSwapChainVtable = (DWORD*)pSwapChainVtable[0];

	pDeviceContextVTable = (DWORD*)pContext;
	pDeviceContextVTable = (DWORD*)pDeviceContextVTable[0];

	Log::Write("Present: 0x" + Log::convertToHexa(pSwapChainVtable[8]));

	if (MH_CreateHook((PVOID*)pSwapChainVtable[8], hD3D11Present, reinterpret_cast<void**>(&phookD3D11Present)) != MH_OK)
		Log::WriteError("Couldn't create D3D11 hook");

	if (MH_EnableHook((PVOID*)pSwapChainVtable[8]) != MH_OK)
		Log::WriteError("Couldn't enable D3D11 hook");

	//phookD3D11Present = (tD3D11Present)DetourFunc((BYTE*)pSwapChainVtable[8], (BYTE*)hD3D11Present, 5);
	//phookD3D11DrawIndexed = (D3D11DrawIndexedHook)DetourFunc((BYTE*)pDeviceContextVTable[12], (BYTE*)hookD3D11DrawIndexed, 5);
	//phookD3D11ClearRenderTargetView = (D3D11ClearRenderTargetViewHook)DetourFunc((BYTE*)pDeviceContextVTable[50], (BYTE*)hookD3D11ClearRenderTargetView, 5);

	DWORD dwOld;
	VirtualProtect(phookD3D11Present, 2, PAGE_EXECUTE_READWRITE, &dwOld);

	pDevice->Release();
	pContext->Release();
	pSwapChain->Release();

	return NULL;
}

int __fastcall hPostProcess(void* This, void* Unused)
{
	int result = oPostProcess(This);

	Main::m_postProcess.PostProcessHook(This);

	return result;
}

unsigned __int8 __fastcall hCamera(void* This, void* Unused)
{
	Main::m_camera.CameraHook(This);
	return oCamera(This);
}

int __fastcall hHelmet(void* This, void* Unused, int a1)
{
	int result = oHelmet(This, a1);
	Main::m_other.helmetHook(This);
	return result;
}

char __fastcall hInput(void* This, void* Unused)
{
	if (!Main::m_camera.inputDisabled(This))
		return oInput(This);
	return 0;
}

int __fastcall hCamera2(void* This, void* Unused, int a1, int a2)
{
	int result = oCamera2(This, a1, a2);

	Main::m_camera.CameraHook3();

	return result;
}

float __fastcall hCamera3(void* This, void* Unused, int a1, int a2, int a3)
{
	float result = oCamera3(This, a1, a2, a3);

	Main::m_camera.CameraHook3();

	return result;
}

int __fastcall hRotation(void* This, void* Unused, void* a2, void* a3, void* a4, void* a5)
{
	Main::m_camera.rotationHook(This);
	//Log::Write("Unused: " + Log::convertToHexa(Unused));
	/*
	int address;

	__asm
	{
		mov[address], edi
	}

	Log::Write("Address: 0x" + Log::convertToHexa(address));
	*/

	return oRotation(This, a2, a3, a4, a5);
}

char __fastcall hPlayer(void* This, void* Unused, int a2)
{
	Main::m_other.PlayerHook(This);
	return oPlayerHook(This, a2);
}

void __fastcall hCharacter(void* This, void* Unused)
{
	Main::m_other.CharacterHook(This);
	return oCharacter(This);
}

char __fastcall hFreeze1(void* This, void* Unused, signed int a1)
{
	Main::m_other.Freeze1Hook(This, Offsets::AI_Freeze2);

	char result = oFreeze1(This, a1);

	int* flag = (int*)((int)Offsets::AI_Freeze2 + 0x4);
	*flag = 0;
	return result;
}

void __fastcall hUI(void* This, void* Unused, int a1)
{
	Main::m_other.UIHook(This);
	return oUI(This, a1);
}

void Hooks::Init()
{
	firstTime = true;
	bool dxHooked = false;

	DWORD dwAppProfile = (DWORD)GetModuleHandle("AppProfiles.dll");
	DWORD dwDXGI = (DWORD)GetModuleHandle("dxgi.dll");
	DWORD dwAliasIsolation = (DWORD)GetModuleHandle("aliasIsolation.dll");

	Log::Write("AppProfiles: 0x" + Log::convertToHexa(dwAppProfile));
	Log::Write("DXGI: 0x" + Log::convertToHexa(dwDXGI));
	Log::Write("aliasIsolation: 0x" + Log::convertToHexa(dwAliasIsolation));


	if (MH_Initialize() != MH_OK)
		Log::WriteError("Couldn't initialize MinHook");

	if (MH_CreateHook((PVOID*)Offsets::AI_PostProcess, hPostProcess, reinterpret_cast<void**>(&oPostProcess)) != MH_OK)
		Log::WriteError("Couldn't create post process hook");
	if (MH_CreateHook((PVOID*)Offsets::AI_Camera, hCamera, reinterpret_cast<void**>(&oCamera)) != MH_OK)
		Log::WriteError("Couldn't create camera hook");
	if (MH_CreateHook((PVOID*)Offsets::AI_Input, hInput, reinterpret_cast<void**>(&oInput)) != MH_OK)
		Log::WriteError("Couldn't create input hook");
	if (MH_CreateHook((PVOID*)Offsets::AI_Rotation, hRotation, reinterpret_cast<void**>(&oRotation)) != MH_OK)
		Log::WriteError("Couldn't create rotation hook");
	if (MH_CreateHook((PVOID*)Offsets::AI_Player, hPlayer, reinterpret_cast<void**>(&oPlayerHook)) != MH_OK)
		Log::WriteError("Couldn't create player hook");
	if (MH_CreateHook((PVOID*)Offsets::AI_Helmet, hHelmet, reinterpret_cast<void**>(&oHelmet)) != MH_OK)
		Log::WriteError("Couldn't create helmet hook");
	if (MH_CreateHook((PVOID*)Offsets::AI_Character, hCharacter, reinterpret_cast<void**>(&oCharacter)) != MH_OK)
		Log::WriteError("Couldn't create helmet hook");
	if (MH_CreateHook((PVOID*)Offsets::AI_Freeze1, hFreeze1, reinterpret_cast<void**>(&oFreeze1)) != MH_OK)
		Log::WriteError("Couldn't create Freeze1 hook");
	if (MH_CreateHook((PVOID*)Offsets::AI_Camera2, hCamera2, reinterpret_cast<void**>(&oCamera2)) != MH_OK)
		Log::WriteError("Couldn't create Camera2 hook");
	if (MH_CreateHook((PVOID*)Offsets::AI_Camera3, hCamera3, reinterpret_cast<void**>(&oCamera3)) != MH_OK)
		Log::WriteError("Couldn't create Camera3 hook");
	if (MH_CreateHook((PVOID*)Offsets::AI_UI, hUI, reinterpret_cast<void**>(&oUI)) != MH_OK)
		Log::WriteError("Couldn't create UI hook");
	if (MH_EnableHook((PVOID*)Offsets::AI_PostProcess) != MH_OK)
		Log::WriteError("Couldn't enable post process hook");
	if (MH_EnableHook((PVOID*)Offsets::AI_Camera) != MH_OK)
		Log::WriteError("Couldn't enable camera hook");
	if (MH_EnableHook((PVOID*)Offsets::AI_Input) != MH_OK)
		Log::WriteError("Couldn't enable input hook");
	if (MH_EnableHook((PVOID*)Offsets::AI_Rotation) != MH_OK)
		Log::WriteError("Couldn't enable rotation hook");
	if (MH_EnableHook((PVOID*)Offsets::AI_Player) != MH_OK)
		Log::WriteError("Couldn't enable player hook");
	if (MH_EnableHook((PVOID*)Offsets::AI_Helmet) != MH_OK)
		Log::WriteError("Couldn't enable helmet hook");
	if (MH_EnableHook((PVOID*)Offsets::AI_Character) != MH_OK)
		Log::WriteError("Couldn't enable character hook");
	if (MH_EnableHook((PVOID*)Offsets::AI_Freeze1) != MH_OK)
		Log::WriteError("Couldn't enable Freeze1 hook");
	if (MH_EnableHook((PVOID*)Offsets::AI_Camera2) != MH_OK)
		Log::WriteError("Couldn't enable Camera2 hook");
	if (MH_EnableHook((PVOID*)Offsets::AI_Camera3) != MH_OK)
		Log::WriteError("Couldn't enable Camera3 hook");
	if (MH_EnableHook((PVOID*)Offsets::AI_UI) != MH_OK)
		Log::WriteError("Couldn't enable UI hook");

	if (dwAliasIsolation != NULL && !dxHooked)
	{
		void *const hookableOverlay_handle = (LPVOID)GetProcAddress((HMODULE)dwAliasIsolation, "aliasIsolation_hookableOverlayRender");
		if (hookableOverlay_handle)
		{
			void* orig;
			if (MH_CreateHook(hookableOverlay_handle, &overlayRenderHook, &orig) == MH_OK && MH_EnableHook(hookableOverlay_handle) == MH_OK) {
				Log::Write("Alias Isolation overlay hooked");
				dxHooked = true;
			}
		}
	}

	if (dwDXGI != NULL && !dxHooked)
	{
		INT64* byteArray = (INT64*)((int)dwDXGI + 0x1770);
		if (*byteArray == 0xEC83F8E483EC8B55)
		{
			dxHooked = true;
			Log::Write("SweetFX detected");
			if (MH_CreateHook((PVOID*)((int)dwDXGI + 0x1770), hD3D11Present, reinterpret_cast<void**>(&phookD3D11Present)) != MH_OK)
				Log::WriteError("Couldn't create D3D11 hook");
			if (MH_EnableHook((PVOID*)((int)dwDXGI + 0x1770)) != MH_OK)
				Log::WriteError("Couldn't enable D3D11 hook");
		}
	}

	if (dwAppProfile != NULL && !dxHooked)
	{
		Log::Write("RadeonPro detected");
		if (MH_CreateHook((PVOID*)((int)dwAppProfile + 0x21EC6), hD3D11Present, reinterpret_cast<void**>(&phookD3D11Present)) != MH_OK)
			Log::WriteError("Couldn't create AppProfile hook");
		if (MH_EnableHook((PVOID*)((int)dwAppProfile + 0x21EC6)) != MH_OK)
			Log::WriteError("Couldn't enable AppProfile hook");
		dxHooked = true;
	}

	Log::Write("Hooks initialized");

	Sleep(100);

	if (!dxHooked)
	{
		HANDLE thread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)InitializeHook, NULL, NULL, NULL);
	}
	Sleep(100);
	if (failed)
	{
		Log::WriteWarning("DX hook failed, entering special Hoodoo_Operator procedure");
		Log::Write("DXGI: 0x" + Log::convertToHexa(dwDXGI));
		DWORD dwPresent = (DWORD)((int)dwDXGI + 0xFAAD);
		Log::Write("DXGI::Present hopefully at 0x" + Log::convertToHexa(dwPresent) + " Checking...");
		BYTE* opCode = (BYTE*)dwPresent;
		if (*opCode == 0xE9)
		{
			Log::Write("Yay, it's there! Hooking it");
			if (MH_CreateHook((PVOID*)dwPresent, hD3D11Present, reinterpret_cast<void**>(&phookD3D11Present)) != MH_OK)
				Log::WriteError("Couldn't create DX hook");
			if (MH_EnableHook((PVOID*)dwPresent) != MH_OK)
				Log::WriteError("Couldn't enable DX hook");
			Log::Write("Should be done :&");
		}
		else
		{
			dwPresent = (DWORD)((int)dwDXGI + 0xFADF);
			Log::WriteWarning("Not there, try alternative 0x" + Log::convertToHexa(dwPresent));
			opCode = (BYTE*)dwPresent;
			if (*opCode == 0x8B)
			{
				if (MH_CreateHook((PVOID*)dwPresent, hD3D11Present, reinterpret_cast<void**>(&phookD3D11Present)) != MH_OK)
					Log::WriteError("Couldn't create DX hook");
				if (MH_EnableHook((PVOID*)dwPresent) != MH_OK)
					Log::WriteError("Couldn't enable DX hook");
				Log::Write("Should be done :&");
			}
			else
			{
				Log::Write("it's still not working");
			}
		}
	}
}
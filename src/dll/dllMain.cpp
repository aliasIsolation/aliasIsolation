#pragma warning(disable: 4996)	// 'sprintf': This function or variable may be unsafe.

#include <windows.h>
#include <cstdio>
#include <functional>
#include <vector>
#include <algorithm>
#include <filesystem>
namespace fs = std::experimental::filesystem;

#include "MinHook.h"

#include "fnTypes.h"
#include "rendering.h"
#include "dllParams.h"
#include "common.h"
#include "injection.h"
#include "crashHandler.h"
#include "settings.h"


SharedDllParams		g_dllParams;
CreateProcessW_t	CreateProcessW_orig = nullptr;


inline bool ends_with(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

inline bool ends_with(std::wstring const & value, std::wstring const & ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}


char g_modulePath[_MAX_PATH];


BOOL WINAPI CreateProcessW_hook(
  LPCTSTR lpApplicationName,
  LPTSTR lpCommandLine,
  LPSECURITY_ATTRIBUTES lpProcessAttributes,
  LPSECURITY_ATTRIBUTES lpThreadAttributes,
  BOOL bInheritHandles,
  DWORD dwCreationFlags,
  LPVOID lpEnvironment,
  LPCTSTR lpCurrentDirectory,
  LPSTARTUPINFOW lpStartupInfo,
  LPPROCESS_INFORMATION lpProcessInformation
)
{
	// HACK: This depends on whether the target app was compiled with UNICODE. Here we assume it wasn't.
	std::wstring appName = lpApplicationName ? (wchar_t*)lpApplicationName : L"";
	std::transform(appName.begin(), appName.end(), appName.begin(), ::towlower);

	//MessageBoxW(NULL, (LPCWSTR)appName.c_str(), (LPCWSTR)lpApplicationName, NULL);

	if (ends_with(appName, L"\\ai.exe") || ends_with(appName, L"\\steam.exe"))
	{
		bool wasSuspended = (dwCreationFlags & CREATE_SUSPENDED) != 0;
		dwCreationFlags |= CREATE_SUSPENDED;

		const BOOL result = CreateProcessW_orig(
			lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles,
			dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	
		if (result)	{
			injectDll(lpProcessInformation->hProcess, g_modulePath);

			if (!wasSuspended)
			{
				// Resume application
				ResumeThread(lpProcessInformation->hThread);
			}
		}

		return result;
	} else {
		return CreateProcessW_orig(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	}
}


HMODULE g_hModule = nullptr;


void enableShaderHooks();
void disableShaderHooks();

// A thread which sits in the background and checks whether the hook should unload.
// It will also monitor the keyboard for Alias Isolation disable/enable hotkeys.
DWORD WINAPI terminationWatchThread(void*)
{
	while (true) {
		SharedDllParams params = getSharedDllParams();

		if (params.terminate) {
			unhookRendering();
			MH_DisableHook(&SetUnhandledExceptionFilter);
			uninstallCrashHandler();
			FreeLibraryAndExitThread(g_hModule, 0);
		}

		for (int i = 0; i < 5; ++i) {
			if ((GetKeyState(VK_CONTROL) & GetKeyState(VK_INSERT) & 0x80u) != 0u) {
				enableShaderHooks();
			} else if ((GetKeyState(VK_CONTROL) & GetKeyState(VK_DELETE) & 0x80u) != 0u) {
				disableShaderHooks();
			}

			Sleep(50);
		}
	}
}

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilter_hook(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
	return nullptr;
}
void* SetUnhandledExceptionFilter_orig = nullptr;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		//MessageBoxA(NULL, "DLL_PROCESS_ATTACH", NULL, NULL);

		MH_CHECK(MH_Initialize());

		// Set our crash handler and prevent the game from overriding it.
		installCrashHandler();
		MH_CHECK(MH_CreateHook(&SetUnhandledExceptionFilter, &SetUnhandledExceptionFilter_hook, &SetUnhandledExceptionFilter_orig));
		MH_CHECK(MH_EnableHook(&SetUnhandledExceptionFilter));

		g_dllParams = getSharedDllParams();
		GetModuleFileNameA(hModule, g_modulePath, sizeof(g_modulePath));
		fs::path settingsFile = fs::path(g_dllParams.aliasIsolationRootDir) / "settings.txt";
		setSettingsFilePath(settingsFile.string().c_str());

		MH_CHECK(MH_CreateHook(&CreateProcessW, &CreateProcessW_hook, (LPVOID*)&CreateProcessW_orig));
		MH_CHECK(MH_EnableHook(&CreateProcessW));

		hookRendering();

		g_hModule = hModule;
		CreateThread(NULL, NULL, &terminationWatchThread, NULL, NULL, NULL);

		break;
	}
	case DLL_PROCESS_DETACH:
		//MessageBoxA(NULL, "DLL_PROCESS_DETACH", NULL, NULL);

		MH_DisableHook(&CreateProcessW);

		unhookRendering();
		break;
	}

	return TRUE;
}


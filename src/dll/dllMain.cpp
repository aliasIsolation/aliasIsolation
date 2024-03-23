#include <windows.h>
#include <cstdio>
#include <functional>
#include <vector>
#include <algorithm>
#include "MinHook.h"
#include "fnTypes.h"
#include "rendering.h"
#include "common.h"
#include "crashHandler.h"
#include "settings.h"
#include "utilities.h"

int __cdecl ovr_Initialize_hook(const void* /*params*/)
{
	// Return an error
	return 1;
}
void* ovr_Initialize_handle = nullptr;

// This will prevent the Oculus runtime from initializing. This mod doesn't support the unofficial OVR mode.
// It would look completely broken, and might also give some trouble depending on what the Oculus SDK does.
void disableOvr()
{
	HMODULE hModule = GetModuleHandleA("AI.exe");
    ovr_Initialize_handle = (LPVOID)GetProcAddress(hModule, "ovr_Initialize");
	if (ovr_Initialize_handle)
	{
		void* orig;
		MH_CHECK(MH_CreateHook(ovr_Initialize_handle, &ovr_Initialize_hook, &orig));
		MH_CHECK(MH_EnableHook(ovr_Initialize_handle));
	}
}

void enableShaderHooks();
void disableShaderHooks();

// A thread which sits in the background and checks whether the hook should unload.
// It will also monitor the keyboard for Alias Isolation disable/enable hotkeys.
DWORD WINAPI terminationWatchThread(void*)
{
	while (true) {
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

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilter_hook(LPTOP_LEVEL_EXCEPTION_FILTER /*lpTopLevelExceptionFilter*/)
{
	return nullptr;
}
void* SetUnhandledExceptionFilter_orig = nullptr;

BOOL APIENTRY DllMain( HMODULE /*hModule*/,
                       DWORD  ul_reason_for_call,
                       LPVOID /*lpReserved*/
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
#ifdef _DEBUG
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
#endif

		MH_CHECK(MH_Initialize());
		LOG_MSG("[aliasIsolation::dllMain] MH_Initialize()\n", "");

		// Set our crash handler and prevent the game from overriding it.
		installCrashHandler();
		MH_CHECK(MH_CreateHook(&SetUnhandledExceptionFilter, &SetUnhandledExceptionFilter_hook, &SetUnhandledExceptionFilter_orig));
		LOG_MSG("[aliasIsolation::dllMain] MH_CreateHook(&SetUnhandledExceptionFilter)\n", "");
		MH_CHECK(MH_EnableHook(&SetUnhandledExceptionFilter));
		LOG_MSG("[aliasIsolation::dllMain] MH_EnableHook(&SetUnhandledExceptionFilter)\n", "");

		std::string settingsFile = getSettingsFilePath();
		LOG_MSG("[aliasIsolation::dllMain] getSettingsFilePath()\n", "");

		setSettingsFilePath(settingsFile.c_str());
		LOG_MSG("[aliasIsolation::dllMain] setSettingsFilePath()\n", "");

		disableOvr();
		LOG_MSG("[aliasIsolation::dllMain] disableOvr()\n", "");

		hookRendering();
		LOG_MSG("[aliasIsolation::dllMain] hookRendering()\n", "");

		HANDLE thread = CreateThread(NULL, NULL, &terminationWatchThread, NULL, NULL, NULL);
		if (thread != nullptr) {
			SetThreadDescription(thread, L"AliasIsolation_TerminationWatchThread");
		}
		LOG_MSG("[aliasIsolation::dllMain] CreateThread(&terminationWatchThread)\n", "");

		break;
	}
	case DLL_PROCESS_DETACH:
		if (ovr_Initialize_handle)
		{
			MH_CHECK(MH_DisableHook(ovr_Initialize_handle));
			LOG_MSG("[aliasIsolation::dllMain] MH_DisableHook(ovr_Initialize_handle)\n", "");
		}

		unhookRendering();
		LOG_MSG("[aliasIsolation::dllMain] unhookRendering()\n", "");
		MH_Uninitialize();
		LOG_MSG("[aliasIsolation::dllMain] MH_Uninitialize()\n", "");

		break;
	}

	return TRUE;
}


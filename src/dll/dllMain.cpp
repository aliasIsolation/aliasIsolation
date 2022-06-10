#pragma warning(disable: 4996)	// 'sprintf': This function or variable may be unsafe.

#include <windows.h>
#include <cstdio>
#include <functional>
#include <vector>
#include <algorithm>
#include <filesystem>
namespace fs = std::filesystem;

#include "MinHook.h"

#include "fnTypes.h"
#include "rendering.h"
#include "common.h"
#include "injection.h"
#include "crashHandler.h"
#include "settings.h"
#include "utilities.h"


std::vector<CreateProcessW_t>	CreateProcessW_orig;
enum { CreateProcessW_MaxOrigFnCount = 32 * 1024 * 1024 };


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

int __cdecl ovr_Initialize_hook(const void* params)
{
	// Return an error
	return 1;
}
void* ovr_Initialize_handle = nullptr;

// This will prevent the Oculus runtime from initializing. This mod doesn't support the unofficial OVR mode.
// It would look completely broken, and might also give some trouble dependong on what the Oculus SDK does.
void disableOvr()
{
	HMODULE hModule = GetModuleHandleA("AI.exe");
	void *const ovr_Initialize_handle = (LPVOID)GetProcAddress(hModule, "ovr_Initialize");
	if (ovr_Initialize_handle)
	{
		void* orig;
		MH_CHECK(MH_CreateHook(ovr_Initialize_handle, &ovr_Initialize_hook, &orig));
		MH_CHECK(MH_EnableHook(ovr_Initialize_handle));
	}
}

/*
TODO! Get Cinematic Tools working again.
void loadCinematicTools()
{
	LoadLibraryA(g_dllParams.cinematicToolsDllPath);
}
*/

char g_modulePath[_MAX_PATH];
char CreateProcessW_hookBytesHead[8];

thread_local int reentranceCountCreateProcessW = 0;

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

	// If we call the "original" function here, if nothing is hooked, it should just call the kernel32 CreateProcessW.
	// However, if we hooked it first, and then some other process replaced the hook, we're trying to be nice and still keep the other
	// hook that ours got replaced by. But now if that other hook was also being nice and calls back to our function, we would have
	// an infinite loop where we would once again call back to the other hook.
	// Instead, we keep a history of the hooks we replaced, and call them based on the re-entrance.
	CreateProcessW_t origFn = CreateProcessW_orig[CreateProcessW_orig.size() - reentranceCountCreateProcessW - 1];

	if (ends_with(appName, L"\\ai.exe") || ends_with(appName, L"\\steam.exe") || ends_with(appName, L"\\epicgameslauncher.exe"))
	{
		bool wasSuspended = (dwCreationFlags & CREATE_SUSPENDED) != 0;
		dwCreationFlags |= CREATE_SUSPENDED;

		reentranceCountCreateProcessW += 1;
		const BOOL result = origFn(
			lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles,
			dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
		reentranceCountCreateProcessW -= 1;

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
		reentranceCountCreateProcessW += 1;
		const BOOL result = origFn(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
		reentranceCountCreateProcessW -= 1;
		return result;
	}
}


void installCreateProcessHook()
{
	MH_RemoveHook(&CreateProcessW);
	void* orig;
	MH_CHECK(MH_CreateHook(&CreateProcessW, &CreateProcessW_hook, &orig));
	MH_CHECK(MH_EnableHook(&CreateProcessW));
	memcpy(CreateProcessW_hookBytesHead, &CreateProcessW, sizeof(CreateProcessW_hookBytesHead));

	if (CreateProcessW_orig.size() < CreateProcessW_MaxOrigFnCount)
	{
		CreateProcessW_orig.push_back((CreateProcessW_t)orig);
	}
}


HMODULE g_hModule = nullptr;


void enableShaderHooks();
void disableShaderHooks();

// A thread which sits in the background and checks whether the hook should unload.
// It will also monitor the keyboard for Alias Isolation disable/enable hotkeys.
DWORD WINAPI terminationWatchThread(void*)
{
	SetThreadDescription(GetCurrentThread(), L"AliasIsolation_TerminationWatchThread");

	while (true) {
		SharedDllParams params = getSharedDllParams();

		if (params.terminate) {
			LOG_MSG("[aliasIsolation::terminationWatchThread] Terminating...\n", "");

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

			// Some nasty DLLs replace the CreateProcessW hook, thus preventing our mod from working.
			// In response, we're being aggressive and check frequently whether our hook is the current one.
			// If not, we install it again.
			if (0 != memcmp(CreateProcessW_hookBytesHead, &CreateProcessW, sizeof(CreateProcessW_hookBytesHead)))
			{
				//MessageBoxA(NULL, "CreateProcessW hook stolen!", NULL, NULL);
				LOG_MSG("[aliasIsolation::terminationWatchThread] CreateProcessW hook stolen!\n", "");
				installCreateProcessHook();
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
#ifdef _DEBUG
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
#endif

		//MessageBoxA(NULL, "DLL_PROCESS_ATTACH", NULL, NULL);

		MH_CHECK(MH_Initialize());
		LOG_MSG("[aliasIsolation::dllMain] MH_Initialize()\n", "");

		// Set our crash handler and prevent the game from overriding it.
		installCrashHandler();
		MH_CHECK(MH_CreateHook(&SetUnhandledExceptionFilter, &SetUnhandledExceptionFilter_hook, &SetUnhandledExceptionFilter_orig));
		LOG_MSG("[aliasIsolation::dllMain] MH_CreateHook(&SetUnhandledExceptionFilter)\n", "");
		MH_CHECK(MH_EnableHook(&SetUnhandledExceptionFilter));
		LOG_MSG("[aliasIsolation::dllMain] MH_EnableHook(&SetUnhandledExceptionFilter)\n", "");

		GetModuleFileNameA(hModule, g_modulePath, sizeof(g_modulePath));
		LOG_MSG("[aliasIsolation::dllMain] GetModuleFileNameA()\n", "");
		
		std::string settingsFile = getSettingsFilePath();
		LOG_MSG("[aliasIsolation::dllMain] getSettingsFilePath()\n", "");

		setSettingsFilePath(settingsFile.c_str());
		LOG_MSG("[aliasIsolation::dllMain] setSettingsFilePath()\n", "");

		installCreateProcessHook();
		LOG_MSG("[aliasIsolation::dllMain] installCreateProcessHook()\n", "");

		disableOvr();
		LOG_MSG("[aliasIsolation::dllMain] disableOvr()\n", "");

		hookRendering();
		LOG_MSG("[aliasIsolation::dllMain] hookRendering()\n", "");

		g_hModule = hModule;
		CreateThread(NULL, NULL, &terminationWatchThread, NULL, NULL, NULL);
		LOG_MSG("[aliasIsolation::dllMain] CreateThread(&terminationWatchThread)\n", "");

		/*
		TODO! Get Cinematic Tools working again.
		if (g_dllParams.cinematicToolsEnable && GetModuleHandleA("AI.exe"))
		{
			loadCinematicTools();
			LOG_MSG("[aliasIsolation::dllMain] loadCinematicTools()\n", "");
		}
		*/

		break;
	}
	case DLL_PROCESS_DETACH:
		//MessageBoxA(NULL, "DLL_PROCESS_DETACH", NULL, NULL);

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


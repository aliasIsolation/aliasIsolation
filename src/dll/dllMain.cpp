#include <Windows.h>
#include <algorithm>
#include <cstdio>
#include <functional>
#include <vector>

#include "MinHook.h"
#include "common.h"
#include "crashHandler.h"
#include "fnTypes.h"
#include "rendering.h"
#include "settings.h"
#include "tracy/Tracy.hpp"
#include "utilities.h"

void enableShaderHooks();
void disableShaderHooks();

// A thread which sits in the background and checks whether the hook should
// unload. It will also monitor the keyboard for Alias Isolation disable/enable
// hotkeys.
DWORD WINAPI terminationWatchThread(void *)
{
    while (true)
    {
        for (int i = 0; i < 5; ++i)
        {
            if ((GetKeyState(VK_CONTROL) & GetKeyState(VK_INSERT) & 0x80u) != 0u)
            {
                enableShaderHooks();
            }
            else if ((GetKeyState(VK_CONTROL) & GetKeyState(VK_DELETE) & 0x80u) != 0u)
            {
                disableShaderHooks();
            }

            Sleep(50);
        }
    }
}

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI
SetUnhandledExceptionFilter_hook(LPTOP_LEVEL_EXCEPTION_FILTER /*lpTopLevelExceptionFilter*/)
{
    return nullptr;
}
void *SetUnhandledExceptionFilter_orig = nullptr;

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD ul_reason_for_call, LPVOID /*lpReserved*/
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
        ZoneScoped;

#ifdef _DEBUG
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
#endif

        MH_CHECK(MH_Initialize());
        LOG_MSG("[aliasIsolation::dllMain] MH_Initialize()\n", "");

        // Set our crash handler and prevent the game from overriding it.
        installCrashHandler();
        MH_CHECK(MH_CreateHook(&SetUnhandledExceptionFilter, &SetUnhandledExceptionFilter_hook,
                               &SetUnhandledExceptionFilter_orig));
        LOG_MSG("[aliasIsolation::dllMain] "
                "MH_CreateHook(&SetUnhandledExceptionFilter)\n",
                "");
        MH_CHECK(MH_EnableHook(&SetUnhandledExceptionFilter));
        LOG_MSG("[aliasIsolation::dllMain] "
                "MH_EnableHook(&SetUnhandledExceptionFilter)\n",
                "");

        std::string settingsFile = getSettingsFilePath();
        LOG_MSG("[aliasIsolation::dllMain] getSettingsFilePath()\n", "");

        setSettingsFilePath(settingsFile.c_str());
        LOG_MSG("[aliasIsolation::dllMain] setSettingsFilePath(settingsFile.c_str())\n", "");

        hookRendering();
        LOG_MSG("[aliasIsolation::dllMain] hookRendering()\n", "");

        HANDLE thread = CreateThread(NULL, NULL, &terminationWatchThread, NULL, NULL, NULL);
        if (thread != nullptr)
        {
            SetThreadDescription(thread, L"AliasIsolation_TerminationWatchThread");
        }
        LOG_MSG("[aliasIsolation::dllMain] CreateThread(&terminationWatchThread)\n", "");

        break;
    }
    case DLL_PROCESS_DETACH:
        ZoneScoped;

        unhookRendering();
        LOG_MSG("[aliasIsolation::dllMain] unhookRendering()\n", "");
        MH_Uninitialize();
        LOG_MSG("[aliasIsolation::dllMain] MH_Uninitialize()\n", "");

        break;
    }

    return TRUE;
}

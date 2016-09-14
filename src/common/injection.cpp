#include "injection.h"
#include <psapi.h>

// Stolen from Robert Kuster's Code Project article: http://www.codeproject.com/Articles/4610/Three-Ways-to-Inject-Your-Code-into-Another-Proces

void injectDll(HANDLE hTarget, const char* const modulePath)
{
	HANDLE  hThread;
	void*   pLibRemote;		// The address (in the remote process) where szLibPath will be copied to;
	DWORD   hLibModule;		// Base address of loaded module (==HMODULE);
	HMODULE hKernel32 = GetModuleHandle(TEXT("Kernel32"));

	const size_t modulePathLen = strlen(modulePath);

	// Allocate remote memory
	pLibRemote = VirtualAllocEx(hTarget, NULL, modulePathLen+1, MEM_COMMIT, PAGE_READWRITE);

	// Copy library name
	WriteProcessMemory(hTarget, pLibRemote, modulePath, modulePathLen+1, NULL);

	// Load dll into the remote process
	hThread = CreateRemoteThread(hTarget, NULL, 0, (LPTHREAD_START_ROUTINE) GetProcAddress(hKernel32, "LoadLibraryA"), pLibRemote, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);

	// Get handle of the loaded module
	GetExitCodeThread(hThread, &hLibModule);

	// Clean up
	CloseHandle(hThread);
	VirtualFreeEx(hTarget, pLibRemote, modulePathLen+1, MEM_RELEASE);
}

bool isAlreadyInjected(const HANDLE proc, const std::string& dllPath)
{
	HMODULE	mods[1024];
	DWORD	bytesNeeded;

	if (EnumProcessModules(proc, mods, sizeof(mods), &bytesNeeded)) {
		for (DWORD i = 0; i < (bytesNeeded / sizeof(HMODULE)); ++i) {
			char modName[MAX_PATH];

			if (GetModuleFileNameExA(proc, mods[i], modName, sizeof(modName))) {
				if (0 == strcmpi(dllPath.c_str(), modName)) {
					return true;
				}
			}
		}
	}

	return false;
}

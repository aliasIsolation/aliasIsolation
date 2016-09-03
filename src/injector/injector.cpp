#pragma warning(disable: 4996)	// 'sprintf': This function or variable may be unsafe.

#include <cstdio>
#include <string>
#include <fstream>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <shlwapi.h>

#include "dllParams.h"
#include "injection.h"

DWORD		getThreadIdFromProcName(const char * ProcName);

std::string	getAliasIsolationDllPath();
bool		getAiSteamInstallPath(std::string *const result);

bool		inject(const std::string& arg);
bool		isAlreadyInjected(const HANDLE proc, const std::string& dllPath);


int main(int argc, char* argv[])
{
	std::string arg;
	if (argc > 1) {
		arg = argv[1];
	} else {
		getline(std::ifstream("args.cfg"), arg);
	}

	if ("detach" == arg) {
		SharedDllParams dllParams;
		ZeroMemory(&dllParams, sizeof(dllParams));
		dllParams.terminate = "detach" == arg;
		setSharedDllParams(dllParams);
		return 0;
	}

	SharedDllParams dllParams;
	ZeroMemory(&dllParams, sizeof(dllParams));
	GetCurrentDirectoryA(sizeof(dllParams.aliasIsolationRootDir), dllParams.aliasIsolationRootDir);
	setSharedDllParams(dllParams);

	if (arg.empty()) {
		std::string installPath;
		if (getThreadIdFromProcName("Steam.exe") && getAiSteamInstallPath(&installPath)) {
			printf("Steam already running! Injecting into it, and starting AI.");
			inject("steam");
			inject(installPath);
			return 0;
		} else if (!getAiSteamInstallPath(&arg)) {
			printf("Could not get the AI install path.\n");
			getchar();
			return 1;
		}
	}
	
	if (inject(arg)) {
		printf("DLL injected!\n");
	} else {
		printf("Failed to inject the dll...\n");
	}

	return 0;
}

bool inject(const std::string& arg)
{
	const std::string dllPath = getAliasIsolationDllPath();

	const char* procName = "AI.exe";

	PROCESS_INFORMATION pi;
	ZeroMemory( &pi, sizeof(pi) );

	if ("steam" == arg) {
		procName = "Steam.exe";
	} else {
		STARTUPINFO si;
		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);

		char binaryPathBuf[1024] = {};
		std::string binaryDir = arg;
		_snprintf(binaryPathBuf, sizeof(binaryPathBuf), "%s\\AI.exe", arg.c_str());

		if (!CreateProcess(
			NULL,
			binaryPathBuf,
			NULL,
			NULL,
			FALSE,
			NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED,
			NULL,
			binaryDir.c_str(),
			&si,
			&pi))
		{
			MessageBoxA(0, "Could not create process...","ERROR",MB_OK);
			return false;
		}
	}
	
	const DWORD threadId = getThreadIdFromProcName(procName);

	if (!threadId) {
		printf("%s process not found\n", procName);
		getchar();
		return false; 
	}

	const HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, threadId); 
	if (!proc) { 
		printf("OpenProcess() failed: %d", GetLastError());
		getchar();
		return false; 
	} 

	if (isAlreadyInjected(proc, dllPath)) {
		printf("DLL already injected!");
		return false;
	}

	injectDll(proc, dllPath.c_str());
	CloseHandle(proc);

	if ("steam" != arg) {
		ResumeThread(pi.hThread);
	}

	return true; 
}

DWORD getThreadIdFromProcName(const char * ProcName)
{
	HANDLE thSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (thSnapShot == INVALID_HANDLE_VALUE) {
		printf("Error: Unable to create toolhelp snapshot!");
		return false;
	}
 
	PROCESSENTRY32 pe;
	ZeroMemory(&pe, sizeof(pe));
	pe.dwSize = sizeof(PROCESSENTRY32);
 
	BOOL retval = Process32First(thSnapShot, &pe);
	while (retval) {
		if(!strcmp(pe.szExeFile, ProcName)) {
			return pe.th32ProcessID;
		}
		retval = Process32Next(thSnapShot, &pe);
	}

	return 0;
}

HMODULE getCurrentModule()
{
	HMODULE hModule = nullptr;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)getCurrentModule, &hModule);
	return hModule;
}

std::string getAliasIsolationDllPath()
{
	char modulePath[_MAX_PATH];
	GetModuleFileNameA(getCurrentModule(), modulePath, sizeof(modulePath));
	PathRemoveFileSpec(modulePath);
	return std::string(modulePath) + "\\aliasIsolation.dll";
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

bool getAiSteamInstallPath(std::string *const result)
{
	HKEY hKey;

	if (ERROR_SUCCESS == RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App 214490", 0, KEY_READ, &hKey))
	{
		char	buf[MAX_PATH];
		DWORD	bufSize = sizeof(buf);
		if (ERROR_SUCCESS == RegQueryValueExA(hKey, "InstallLocation", 0, nullptr, (LPBYTE)buf, &bufSize)) {
			*result = buf;
			return true;
		}
	}

	return false;
}

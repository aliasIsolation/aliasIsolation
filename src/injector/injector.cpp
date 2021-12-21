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
#include <boost/bind/bind.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#define BOOST_BIND_GLOBAL_PLACEHOLDERS

DWORD		getThreadIdFromProcName(const char * ProcName);

std::string	getAliasIsolationDllPath();
bool		getAiSteamInstallPath(std::string *const result);
bool		getAiEGSInstallPath(std::string* const result);

bool		inject(const std::string& arg);


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
		dllParams.terminate = true;
		setSharedDllParams(dllParams);

		// Reset the terminate flag back to false again so we don't terminate on next launch.
		//dllParams.terminate = false;
		//setSharedDllParams(dllParams);
		return 0;
	}

	SharedDllParams dllParams;
	ZeroMemory(&dllParams, sizeof(dllParams));
	GetCurrentDirectoryA(sizeof(dllParams.aliasIsolationRootDir), dllParams.aliasIsolationRootDir);
	setSharedDllParams(dllParams);

	if (arg.empty()) {
		std::string installPath;
		if (getAiSteamInstallPath(nullptr) && getAiEGSInstallPath(nullptr)) {
			printf_s("Detected two installations of Alien: Isolation (Steam and Epic Games Store), please choose which installation you would like to launch by running this injector with the \"-steam\" or \"-egs\" parameter.\n");
			printf_s("For example: Create a shortcut pointing to \"aliasIsolationInjector.exe -steam\" to launch the Steam version of Alien: Isolation.\n");
			getchar();
			return 1;
		}
		else if (getThreadIdFromProcName("Steam.exe") && getAiSteamInstallPath(&installPath)) {
			printf("Steam already running! Injecting into it, and starting AI.");
			inject("steam");
			inject(installPath);
			return 0;
		} 
		else if (getThreadIdFromProcName("EpicGamesLauncher.exe") && getAiEGSInstallPath(&installPath)) {
			printf_s("Epic Games Launcher already running! Injecting into it, and starting AI.");
			inject("epicgameslauncher");
			inject(installPath);
			return 0;
		}
		else if (!getAiSteamInstallPath(&arg) && !getAiEGSInstallPath(&arg)) {
			printf("Could not get the AI install path.\n");
			getchar();
			return 1;
		}
	}
	else
	{
		// Check if the user has specified that they want to launch a specific version of the game.
		if (!arg.compare("-steam"))
		{
			getAiSteamInstallPath(&arg);
		}
		else if (!arg.compare("-egs"))
		{
			getAiEGSInstallPath(&arg);
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
	}
	else if ("epicgameslauncher" == arg) {
		procName = "EpicGamesLauncher.exe";
	}
	else {
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

bool getAiSteamInstallPath(std::string *const result)
{
	HKEY hKey;
	const char* keysToTry[] = {
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App 214490",
		"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App 214490",
	};

	for (const char* keyName : keysToTry)
	{
		LSTATUS status = RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyName, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey);
		if (ERROR_SUCCESS == status)
		{
			char	buf[MAX_PATH];
			DWORD	bufSize = sizeof(buf);
			if (ERROR_SUCCESS == RegQueryValueExA(hKey, "InstallLocation", 0, nullptr, (LPBYTE)buf, &bufSize)) {
				// Allow passing a nullptr to see if we were able to get the AI Steam install path, but do not try to write data to it.
				if (result != nullptr)
				{
					*result = buf;
				}

				return true;
			}
		}
	}

	return false;
}

bool getAiEGSInstallPath(std::string *const result)
{
	std::string egsLauncherInstalledPath = "C:\\ProgramData\\Epic\\UnrealEngineLauncher\\LauncherInstalled.dat";

	boost::property_tree::ptree pt;
	boost::property_tree::read_json(egsLauncherInstalledPath, pt);

	// Iterate through each EGS game install element.
	for (auto& installation : pt.get_child("InstallationList")) {
		// Iterate through each install's property elements.
		for (auto& installationInfo : installation.second) {
			// Are we dealing with the install location element?
			if (!installationInfo.first.compare("InstallLocation"))
			{
				std::string installLocation = installationInfo.second.get_value<std::string>();

				// Make sure this install location contains the string "AlienIsolation", this is the hardcoded part of the path and is easy to identify.
				if (installLocation.find("AlienIsolation") != std::string::npos)
				{
					// Allow passing a nullptr to see if we were able to get the game's EGS install path, but do not try to write data to it.
					if (result != nullptr)
					{
						// Set the value of result to the path to Alien: Isolation's EGS directory.
						*result = installLocation;
					}

					return true;
				}
			}
		}
	}

	return false;
}

#pragma warning(disable: 4996)	// 'sprintf': This function or variable may be unsafe.

#include <cstdio>
#include <string>
#include <fstream>
#define NOMINMAX
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <shlwapi.h>
#include <filesystem>
#include <algorithm>
namespace fs = std::experimental::filesystem;

#include "dllParams.h"
#include "injection.h"
#include "crashHandler.h"
#include "settings.h"

#include <nana/gui.hpp>
#include <nana/gui/widgets/group.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/filebox.hpp>

std::string	getAliasIsolationDllPath();
bool		getAiSteamInstallPath(std::string *const result);

void	onSettingsChanged(const Settings& settings);
void	fatalError(const char* lpszFunction);
void	detach();
void	onInject(const std::string& exePath);
bool	startSuspendedProcess(const std::string& procPath, PROCESS_INFORMATION *const pi);
DWORD	findProcess(const std::string& procName);
bool	injectIntoProcess(const DWORD procId);


int CALLBACK WinMain(
	_In_ HINSTANCE,
	_In_ HINSTANCE,
	_In_ LPSTR,
	_In_ int
)
{
	installCrashHandler();

	Settings settings;
	setSettingsFilePath("settings.txt");
	loadSettings(&settings);

	using namespace nana;
	form fm{API::make_center(500, 400)};
	fm.caption(("Alias Isolation"));
	place plc(fm);

	group gameDirGroup(fm);
	gameDirGroup.caption("Game directory");
	gameDirGroup.div("vert gap=5 margin=10 <caption weight=60><path weight=22 gap=5 arrange=[variable,100]>");

	auto lab1 = gameDirGroup.create_child<label>("caption");
	lab1->format(true);

	auto pathTextbox = gameDirGroup.create_child<textbox>("path");

	std::string steamAiInstallNotFoundText = "<color=0xa00000>The installation directory of Alien: Isolation could not be automatically determined.</>\nPlease use the field below to locate the game's main executable (<bold>AI.exe</>).";
	std::string aiExePath = settings.aiExePath;

	auto isInstallPathValid = [&]{
		return fs::exists(aiExePath) && fs::path(aiExePath).filename().compare("AI.exe") == 0;
	};

	if (isInstallPathValid()) {
		lab1->caption("Alien: Isolation located.");
		pathTextbox->caption(aiExePath);
	} else if (getAiSteamInstallPath(&aiExePath)) {
		aiExePath += "\\AI.exe";
		lab1->caption("A Steam installation of Alien: Isolation was found. You don't need to adjust the path.");
		pathTextbox->caption(aiExePath);
	} else {
		lab1->caption(steamAiInstallNotFoundText);
		pathTextbox->caption("C:\\path\\to\\alien\\AI.exe");
	}

	auto browseButton = gameDirGroup.create_child<button>("path");
	browseButton->caption("Browse...");

	filebox fb(true);
	fb.add_filter("Alien Isolation", "AI.exe");

	browseButton->events().click([&] {
		if (fb()) {
			aiExePath = fb.file();
			pathTextbox->caption(aiExePath);
		}
	});

	pathTextbox->events().text_changed([&] {
		aiExePath = pathTextbox->caption();
		if (isInstallPathValid()) {
			lab1->caption("Alien: Isolation located.");
			settings.aiExePath = aiExePath;
			onSettingsChanged(settings);
		} else {
			lab1->caption(steamAiInstallNotFoundText);
		}
	});

	group settingsGroup(fm);
	settingsGroup.caption("Settings");
	settingsGroup.div("vert gap=5 margin=10 list arrange=[30,repeated]");

	auto createFloatSetting = [&](const char* name, float *const targetSetting)
	{
		panel<false>& settingPanel = *settingsGroup.create_child<panel<false>>("list");
		place& settingPlace = *new place;
		settingPlace.bind(settingPanel);
		settingPlace.div("<label weight=130><slider><text weight=50>");

		label& settingLabel = *new label(settingPanel);
		settingPlace["label"] << settingLabel;
		settingLabel.caption(name);
		settingLabel.text_align(align::left, align_v::center);

		textbox& settingText = *new textbox(settingPanel);
		settingText.caption(std::to_string(*targetSetting));
		settingText.multi_lines(false);
		settingText.set_accept([](wchar_t c){
			return (c >= '0' && c <= '9') || c == '.' || c == '\b';
		});
		settingPlace["text"] << settingText;

		slider& settingAmountSlider = *new slider(settingPanel);
		settingPlace["slider"] << settingAmountSlider;
		settingAmountSlider.create(fm);
		settingAmountSlider.maximum(1000);
		settingAmountSlider.value(int(*targetSetting * 1000));
		settingAmountSlider.vernier([=](unsigned maximum, unsigned cursor_value)
		{
			return std::to_string(100 * cursor_value / maximum) + "%";
		});
		settingAmountSlider.events().value_changed([&settings, &settingText, targetSetting](const arg_slider& arg){
			float val = float(arg.widget.value()) / arg.widget.maximum();
			*targetSetting = val;
			settingText.caption(std::to_string(*targetSetting));
			onSettingsChanged(settings);
		});
		settingText.events().text_changed([&settings, &settingAmountSlider, targetSetting](const arg_textbox& arg){
			float val = (float)atof(arg.widget.caption().c_str());
			if (val < 0.0f || val > 1.0f) {
				val = std::max(0.0f, std::min(1.0f, val));
				arg.widget.caption(std::to_string(val));
			}

			*targetSetting = val;
			settingAmountSlider.value(int(1000 * val));
			onSettingsChanged(settings);
		});
	};

	createFloatSetting("Sharpening", &settings.sharpening);
	createFloatSetting("Chromatic aberration", &settings.chromaticAberration);

	label infoLabel(fm);
	infoLabel.format(true);
	infoLabel.caption(
		"Please note that this mod requires certain video settings to be configured in the game:\n"
		"* Anti-aliasing must be set to <bold>SMAA T1x</>\n"
		"* Chromatic aberration must be <bold>disabled</>\n"
		"* Motion blur must be <bold>enabled</>"
	);

	button launchButton(fm);
	launchButton.caption("Launch Alien: Isolation");
	launchButton.events().click([&](const arg_click&){
		if (isInstallPathValid()) {
			onInject(aiExePath);
		}
	});

	plc.div("vert margin=15 gap=15 <vert main gap=15 arrange=[120, 100]><<><launchButton weight=70%><> weight=30>");
	plc.field("main") << gameDirGroup << settingsGroup << infoLabel;
	plc.field("launchButton") << launchButton;
	plc.collocate();

	fm.show();

	exec();
	detach();

	return 0;
}

void onSettingsChanged(const Settings& settings)
{
	saveSettings(settings);
}

void onInject(const std::string& exePathString)
{
	{
		SharedDllParams dllParams;
		ZeroMemory(&dllParams, sizeof(dllParams));
		GetCurrentDirectoryA(sizeof(dllParams.aliasIsolationRootDir), dllParams.aliasIsolationRootDir);
		//const fs::path dllDir = fs::path(getAliasIsolationDllPath()).parent_path();
		//const std::string dllDirStr = dllDir.string();
		//strncpy(dllParams.symbolSearchPath, dllDirStr.c_str(), sizeof(dllParams.symbolSearchPath)-1);
		setSharedDllParams(dllParams);
	}

	fs::path exePath(exePathString);
	fs::path aiInstallDir = exePath.parent_path();

	const bool isSteamVersion = fs::exists(aiInstallDir / "STEAM_API.DLL");

	if (isSteamVersion)
	{
		const DWORD steamProcId = findProcess("Steam.exe");
		if (steamProcId)
		{
			injectIntoProcess(steamProcId);
		}
	}

	PROCESS_INFORMATION gameProcInfo;
	if (startSuspendedProcess(exePathString, &gameProcInfo))
	{
		injectIntoProcess(gameProcInfo.dwProcessId);
		ResumeThread(gameProcInfo.hThread);
	}
}

bool startSuspendedProcess(const std::string& procPath, PROCESS_INFORMATION *const pi)
{
	ZeroMemory(pi, sizeof(*pi));

	STARTUPINFO si;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);

	char binaryPathBuf[1024] = {};
	strncpy(binaryPathBuf, procPath.c_str(), sizeof(binaryPathBuf)-1);

	const fs::path workingDirPath = fs::u8path(procPath).parent_path();
	const std::string workingDir = workingDirPath.string();

	if (CreateProcessA(
		NULL,
		binaryPathBuf,
		NULL,
		NULL,
		FALSE,
		NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED,
		NULL,
		workingDir.c_str(),
		&si,
		pi))
	{
		return true;
	}
	else
	{
		fatalError("CreateProcess");
		return false;
	}
}

bool injectIntoProcess(const DWORD procId)
{
	const HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId); 
	if (!proc) { 
		fatalError("OpenProcess");
		//printf("OpenProcess() failed: %d", GetLastError());
		return false; 
	} 

	const std::string dllPath = getAliasIsolationDllPath();
	if (isAlreadyInjected(proc, dllPath)) {
		CloseHandle(proc);
		//MessageBox(NULL, "DLL already injected!", "Error", MB_OK); 
		return false;
	}

	injectDll(proc, dllPath.c_str());
	CloseHandle(proc);

	return true; 
}

DWORD findProcess(const std::string& procName)
{
	HANDLE thSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (thSnapShot == INVALID_HANDLE_VALUE) {
		fatalError("CreateToolhelp32Snapshot");
		return false;
	}
 
	PROCESSENTRY32 pe;
	ZeroMemory(&pe, sizeof(pe));
	pe.dwSize = sizeof(PROCESSENTRY32);
 
	BOOL retval = Process32First(thSnapShot, &pe);
	while (retval) {
		if(!strcmp(pe.szExeFile, procName.c_str())) {
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
				*result = buf;
				return true;
			}
		}
	}

	return false;
}

void detach()
{
	SharedDllParams dllParams;
	ZeroMemory(&dllParams, sizeof(dllParams));
	dllParams.terminate = true;
	setSharedDllParams(dllParams);
}

#include <strsafe.h>
void fatalError(const char* lpszFunction) 
{ 
	// Retrieve the system error message for the last-error code

	char* lpMsgBuf;
	char* lpDisplayBuf;
	DWORD dw = GetLastError(); 

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, NULL );

	// Display the error message and exit the process

	lpDisplayBuf = (char*)LocalAlloc(LMEM_ZEROINIT, 
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
	sprintf(lpDisplayBuf, "%s failed with error %d: %s", lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);

	//ExitProcess(dw); 
	DebugBreak();
}

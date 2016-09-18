#include "Main.h"
#include "MinHook.h"
#include <map>
#include <string>
#include <fstream>
#include <windows.h>
#include <sstream>
#include "Tools\Log.h"
#include "Offsets.h"
#include "Hooks.h"
#include "boost\chrono.hpp"
//#include "Menu.h"

// 10 9B F3 00

typedef void(__thiscall* tReduceHealth1)(void*, int);
typedef void(__thiscall* tPause)(void*);
typedef int(__stdcall* tFreecam)(int);

tFreecam oFreecam = (tFreecam)0x955FE0;
tReduceHealth1 oReduceHealth2 = (tReduceHealth1)0x475280;
tPause oPause2 = (tPause)0x466A50;

using namespace boost::chrono;

Camera Main::m_camera;
PostProcess Main::m_postProcess;
Other Main::m_other;
Menu Main::m_menu;
high_resolution_clock Clock;
high_resolution_clock::time_point prevTime;
int Main::color = 0xFFFFFFFF;
int increment = 0x01000000;
XMFLOAT2 Main::position = XMFLOAT2(100, 100);


void Main::Init()
{
	Log::Init(Log::DebugMode::CONSOLE);
	Sleep(100);
	Offsets::Init();
	m_camera.Init();
	Sleep(100);
	Hooks::Init();

	printf("\n");

	Log::Write("INSERT to toggle menu");
	Log::Write("UP and DOWN to navigate");
	Log::Write("ENTER or RIGHT SHIFT to toggle or change value");
	Log::Write("+ and - or PGUP and PGDOWN to change value ");
	Log::Write("BACKSPACE to return to previous menu");
	Log::Write("Use numpad to rotate the camera when game is frozen");

	printf("\n");

	//ConsoleMenu::Init();

	prevTime = Clock.now();

	HANDLE thread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)m_menu.Update, NULL, NULL, NULL);
	CloseHandle(thread);

	while (1)
	{
		Update();
		//ConsoleMenu::Update();
		Sleep(1);
	}
}

double lastUpdate = 0;

void Main::Update()
{
	duration<double> dt = Clock.now() - prevTime;
	prevTime = Clock.now();

	//Log::Write("0x" + Log::convertToHexa(color));

	/*
	if (GetAsyncKeyState(VK_NEXT) & 0x8000)
	{
		m_other.biggerIndex();
		Sleep(200);
	}
	if (GetAsyncKeyState(VK_PRIOR) & 0x8000)
	{
		m_other.lowerIndex();
		Sleep(200);
	}*/

	m_camera.Update(dt.count());
}
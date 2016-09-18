#pragma once
#include <Windows.h>

class Offsets
{
public:
	static void Init();
	
	static void* AI_Camera;
	static void* AI_Camera2;
	static void* AI_Camera3;
	static void* AI_PostProcess;
	static void* AI_Input;
	static void* AI_Rotation;
	static bool* AI_Focus;
	static void* AI_Player;
	static void* AI_Time;
	static void* AI_Helmet;
	static void* AI_Character;
	static void* AI_TypeInfo_Character;
	static void* AI_Resolution;
	static void* AI_Freeze1;
	static void* AI_Freeze2;
	static void* AI_FreezeGame;
	static void* AI_UI;

	static bool m_initialized;
};
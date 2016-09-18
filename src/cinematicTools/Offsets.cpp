#include "Offsets.h"
#include "Tools\Log.h"

#define READABLE (PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY | PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY)

void* Offsets::AI_Camera;
void* Offsets::AI_Camera2;
void* Offsets::AI_Camera3;
void* Offsets::AI_PostProcess;
void* Offsets::AI_Input;
void* Offsets::AI_Rotation;
bool* Offsets::AI_Focus;
void* Offsets::AI_Player;
void* Offsets::AI_Time;
void* Offsets::AI_Helmet;
void* Offsets::AI_Character;
void* Offsets::AI_TypeInfo_Character;
void* Offsets::AI_Resolution;
void* Offsets::AI_Freeze1;
void* Offsets::AI_Freeze2;
void* Offsets::AI_FreezeGame;
void* Offsets::AI_UI;

bool Offsets::m_initialized = false;

unsigned char* FindPattern(const unsigned char* haystack, size_t hlen,
	const unsigned char* needle, const char* mask)
{
	size_t scan, nlen = strlen(mask);
	size_t bad_char_skip[256];

	for (scan = 0; scan < 256; scan++)
		bad_char_skip[scan] = nlen;

	size_t last = nlen - 1;

	for (scan = 0; scan < last; scan++)
		if (mask[scan] != '?')
			bad_char_skip[needle[scan]] = last - scan;

	while (hlen >= nlen)
	{
		for (scan = last; mask[scan] == '?' || haystack[scan] == needle[scan]; scan--)
			if (scan == 0)
				return (unsigned char*)haystack;

		// This is buggy, and breaks Boyer-Moore. Concretely, AI_Camera3 wasn't being found, as this code was skipping
		// over it. Another search algorithm should be used. In the meantime, brute force.
		/*hlen -= bad_char_skip[haystack[last]];
		haystack += bad_char_skip[haystack[last]];*/

		--hlen;
		++haystack;
	}

	return 0;
}

void Offsets::Init()
{
	HANDLE handle = GetModuleHandleA("AI.exe");
	Log::Write("Base: 0x" + Log::convertToHexa((void*)handle));

	INT scanStart = (int)handle;

	unsigned char camera_Pattern[] = "\x83\xEC\x0C\x53\x56\x8B\xF1\x0F\xB6\x86";
	char camera_Mask[] = "xxxxxxxxxx";

	unsigned char camera2_Pattern[] = "\x81\xEC\x00\x00\x00\x00\x56\x8B\xF1\x8B\x46\x34";
	char camera2_Mask[] = "xx????xxxxxx";

	unsigned char camera3_Pattern[] = "\x81\xEC\x00\x00\x00\x00\x53\x56\x57\x8B\xBC\x24\x00\x00\x00\x00\xF3\x0F\x10\x07";
	char camera3_Mask[] = "xx????xxxxxx????xxxx";

	unsigned char helmet_Pattern[] = "\x55\x8B\xEC\x83\xE4\xF0\x83\xEC\x48\x56\x8B\xF1\x0F\xB7\x46\x34";
	char helmet_Mask[] = "xxxxxxxxxxxxxxxx";

	unsigned char postProcess_Pattern[] = "\x51\xF3\x0F\x7E\x05";
	char postProcess_Mask[] = "xxxxx";

	unsigned char focus_Pattern[] = "\xC6\x05\x00\x00\x00\x00\x01\x33\xC0\x5D\x83\xC4\x38";
	char focus_Mask[] = "xx????xxxxxx";

	unsigned char input_Pattern[] = "\x0F\x57\xED\x56\x8B\xB1";
	char input_Mask[] = "xxxxxx";

	unsigned char rotation_Pattern[] = "\x55\x8B\xEC\x83\xE4\xF0\x83\xEC\x44\x53\x56\x8B\x75\x0C\x8B\xD9\x57\x8D\x93";
	char rotation_Mask[] = "xxxxxxxxxxxxxxxxxxx";

	unsigned char player_Pattern[] = "\xC2\x04\x00\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x83\xEC\x08\x53\x56\x57\x8B\xF1\xE8";
	char player_Mask[] = "xxxxxxxxxxxxxxxxxxxxx";

	unsigned char character_Pattern[] = "\x5F\x5E\x5D\x32\xC0\x5B\xC2\x08\x00\x5F\x5E\x5D\xB0\x01\x5B\xC2\x08\x00\xCC\x8B\xC1\xC3\xCC";
	char character_Mask[] = "xxxxxxxxxxxxxxxxxxxxx";

	unsigned char time_Pattern[] = "\xDC\x05\x00\x00\x00\x00\xDC\x26\xDC\x66\x50";
	char time_Mask[] = "xx????xxxxx";

	unsigned char typeInfoCharacter_Pattern[] = "\x57\x89\x74\x24\x0C\xC7\x06\x00\x00\x00\x00\xC7\x86\x00\x00\x00\x00\x00\x00\x00\x00\x83\xBE";
	char typeInfoCharacter_Mask[] = "xxxxxxx????xx????????xx";

	unsigned char resolution_Pattern[] = "\x56\x8B\xF1\x80\x7E\x29\x00\x0F\x85";
	char resolution_Mask[] = "xxxxxxxxx";

	unsigned char freeze1_Pattern[] = "\x55\x8B\xEC\x83\xE4\xF0\xA1\x00\x00\x00\x00\x81\xEC\x00\x00\x00\x00\x53\x56\x8B\xF1";
	char freeze1_Mask[] = "xxxxxxx????xx????xxxx";

	unsigned char freezeGame_Pattern[] = "\xA1\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\x85\x48\x04\x75\x33";
	char freezeGame_Mask[] = "x????xx????xxxxx";

	unsigned char ui_Pattern[] = "\x56\x8B\xF1\x80\x7E\x29\x00\x75\x6A";
	char ui_Mask[] = "xxxxxxxxx";

	MEMORY_BASIC_INFORMATION memInfo;

	Sleep(100);

	//AI_Camera3 = (char*)scanStart + 0x2bc50;

	while (scanStart < 0xF000000 && (!AI_Camera || !AI_PostProcess || !AI_Input || !AI_Rotation || !AI_Focus || !AI_Player || 
		!AI_Time || !AI_Helmet || !AI_TypeInfo_Character || !AI_Resolution || !AI_Freeze1 || !AI_FreezeGame || !AI_UI))
	{
		VirtualQuery((void*)scanStart, &memInfo, sizeof(MEMORY_BASIC_INFORMATION));
		if (memInfo.State & MEM_COMMIT)
		{
			if (memInfo.AllocationProtect & READABLE)
			{
				if ((INT)memInfo.BaseAddress + (INT)memInfo.RegionSize > 0xF000000)
					break;
				while (scanStart <= (INT)memInfo.BaseAddress + (INT)memInfo.RegionSize - 0x100)
				{
					//Log::Write("0x" + Log::convertToHexa(scanStart));
					if (IsBadCodePtr(reinterpret_cast<FARPROC>(scanStart)) != TRUE && IsBadCodePtr(reinterpret_cast<FARPROC>(scanStart + 0x100)) != TRUE)
					{
						unsigned char* result = FindPattern((const unsigned char*)scanStart, 0x100, camera_Pattern, camera_Mask);
						unsigned char* result2 = FindPattern((const unsigned char*)scanStart, 0x100, postProcess_Pattern, postProcess_Mask);
						unsigned char* result3 = FindPattern((const unsigned char*)scanStart, 0x100, input_Pattern, input_Mask);
						unsigned char* result4 = FindPattern((const unsigned char*)scanStart, 0x100, rotation_Pattern, rotation_Mask);
						unsigned char* result5 = FindPattern((const unsigned char*)scanStart, 0x100, focus_Pattern, focus_Mask);
						unsigned char* result6 = FindPattern((const unsigned char*)scanStart, 0x100, player_Pattern, player_Mask);
						unsigned char* result7 = FindPattern((const unsigned char*)scanStart, 0x100, time_Pattern, time_Mask);
						unsigned char* result8 = FindPattern((const unsigned char*)scanStart, 0x100, helmet_Pattern, helmet_Mask);
						unsigned char* result9 = FindPattern((const unsigned char*)scanStart, 0x100, character_Pattern, character_Mask);
						unsigned char* result10 = FindPattern((const unsigned char*)scanStart, 0x100, typeInfoCharacter_Pattern, typeInfoCharacter_Mask);
						unsigned char* result11 = FindPattern((const unsigned char*)scanStart, 0x100, resolution_Pattern, resolution_Mask);
						unsigned char* result12 = FindPattern((const unsigned char*)scanStart, 0x100, freeze1_Pattern, freeze1_Mask);
						unsigned char* result13 = FindPattern((const unsigned char*)scanStart, 0x100, freezeGame_Pattern, freezeGame_Mask);
						unsigned char* result14 = FindPattern((const unsigned char*)scanStart, 0x100, camera2_Pattern, camera2_Mask);
						unsigned char* result15 = FindPattern((const unsigned char*)scanStart, 0x100, camera3_Pattern, camera3_Mask);
						unsigned char* result16 = FindPattern((const unsigned char*)scanStart, 0x100, ui_Pattern, ui_Mask);
						if (result != NULL)
							AI_Camera = (void*)result;
						if (result2 != NULL)
							AI_PostProcess = (void*)result2;
						if (result3 != NULL)
							AI_Input = (void*)result3;
						if (result4 != NULL)
							AI_Rotation = (void*)result4;
						if (result5 != NULL)
							AI_Focus = *(bool**)((int)result5+0x2);
						if (result6 != NULL)
							AI_Player = (void*)((int)result6 + 0xC);
						if (result7 != NULL)
							AI_Time = *(void**)((int)result7 + 0x2);
						if (result8 != NULL)
							AI_Helmet = (void*)result8;
						if (result9 != NULL)
							AI_Character = (void *)((int)result9 + 0x13);
						if (result10 != NULL)
							AI_TypeInfo_Character = *(void**)((int)result10 + 0x7);
						if (result11 != NULL)
							AI_Resolution = *(void**)((int)result11 + 0xF);
						if (result12 != NULL)
						{
							AI_Freeze1 = (void*)result12;
							AI_Freeze2 = **(void***)((int)result12 + 0x7);
						}
						if (result13 != NULL)
							AI_FreezeGame = **(void***)((int)result13 + 0x1);
						if (result14 != NULL)
							AI_Camera2 = (void*)result14;
						if (result15 != NULL)
							AI_Camera3 = (void*)result15;
						if (result16 != NULL)
							AI_UI = (void*)result16;
					}

					scanStart += 0x50;
				}
				Sleep(10);
			}
		}
		scanStart = (INT)memInfo.BaseAddress + (INT)memInfo.RegionSize;
		//printf("%X\n", scanStart);
	}

	Log::Write("AI_Camera: 0x" + Log::convertToHexa(AI_Camera));
	Log::Write("AI_Camera2: 0x" + Log::convertToHexa(AI_Camera2));
	Log::Write("AI_Camera3: 0x" + Log::convertToHexa(AI_Camera3));
	Log::Write("AI_Helmet: 0x" + Log::convertToHexa(AI_Helmet));
	Log::Write("AI_Character: 0x" + Log::convertToHexa(AI_Character));
	Log::Write("AI_TypeInfo_Character: 0x" + Log::convertToHexa(AI_TypeInfo_Character));
	Log::Write("AI_Input: 0x" + Log::convertToHexa(AI_Input));
	Log::Write("AI_PostProcess: 0x" + Log::convertToHexa(AI_PostProcess));
	Log::Write("AI_Rotation: 0x" + Log::convertToHexa(AI_Rotation));
	Log::Write("AI_Focus: 0x" + Log::convertToHexa(AI_Focus));
	Log::Write("AI_Player: 0x" + Log::convertToHexa(AI_Player));
	Log::Write("AI_Time: 0x" + Log::convertToHexa(AI_Time));
	Log::Write("AI_Resolution: 0x" + Log::convertToHexa(AI_Resolution));
	Log::Write("AI_Freeze1: 0x" + Log::convertToHexa(AI_Freeze1));
	Log::Write("AI_Freeze2: 0x" + Log::convertToHexa(AI_Freeze2));
	Log::Write("AI_FreezeGame: 0x" + Log::convertToHexa(AI_FreezeGame));
	Log::Write("AI_UI: 0x" + Log::convertToHexa(AI_UI));
}

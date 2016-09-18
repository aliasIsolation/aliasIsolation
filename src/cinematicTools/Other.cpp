#include "Other.h"
#include "Offsets.h"
#include "Tools\Log.h"
#include <boost\algorithm\string\predicate.hpp>

const BYTE origHelmetCode[] = { 0xD9, 0x5E, 0x0C, 0xD9, 0x40, 0x04, 0xD9, 0x5E, 0x10, 0xD9, 0x40, 0x14, 0xD9,
								0x5E, 0x14, 0xD9, 0x40, 0x24, 0xD9, 0x5E, 0x18, 0xD9, 0x40, 0x34, 0xD9, 0x5E, 
								0x1C};

const BYTE nopHelmetCode[] = { 0x90, 0x90, 0x90, 0xD9, 0x40, 0x04, 0xD9, 0x5E, 0x10, 0xD9, 0x40, 0x14, 0xD9,
								0x5E, 0x14, 0xD9, 0x40, 0x24, 0xD9, 0x5E, 0x18, 0xD9, 0x40, 0x34, 0x90, 0x90, 
								0x90};

BOOL WriteMemory(DWORD_PTR dwAddress, const void* cpvPatch, DWORD dwSize)
{

	DWORD dwProtect;

	if (VirtualProtect((void*)dwAddress, dwSize, PAGE_READWRITE, &dwProtect)) //Unprotect the memory

		memcpy((void*)dwAddress, cpvPatch, dwSize); //Write our patch

	else

		return false; //Failed to unprotect, so return false..



	return VirtualProtect((void*)dwAddress, dwSize, dwProtect, new DWORD); //Reprotect the memory

}

void Other::PlayerHook(void* This)
{
	pPlayer = (Player*)This;
}

void Other::toggleHelmet()
{
	m_helmetRemoved = !m_helmetRemoved;
}

std::vector<int> items;
int selectedID;
int selectedIndex;

void Other::lowerIndex()
{
	selectedIndex -= 1;
	if (selectedIndex < 0 || selectedIndex > items.size() - 1)
		selectedIndex = items.size() - 1;

	if (selectedIndex > -1)
		selectedID = items.at(selectedIndex);

	Log::Write("ID: " + to_string(selectedID));
}

void Other::biggerIndex()
{
	selectedIndex += 1;
	if (selectedIndex > items.size() - 1)
		selectedIndex = 0;

	if (selectedIndex > -1)
		selectedID = items.at(selectedIndex);
	Log::Write("ID: " + to_string(selectedID));
}

void Other::helmetHook(void* This)
{
	if (m_helmetRemoved)
	{
		int* ID = (int*)((int)This + 0x30);
		/*
		bool found = false;
		for (std::vector<int>::iterator itr = items.begin(); itr != items.end(); ++itr)
		{
			if (*itr == *ID)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			items.push_back(*ID);
		}
		*/

		if (*ID != 974867 && *ID != 983059 && *ID != 1007635 && *ID != 1015827 && *ID != 1122325 && *ID != 81936 && *ID != 90128 &&
			*ID != 49168 && *ID != 32784 && *ID != 40976 && *ID != 1499155 && *ID != 1490963 && *ID != 1359891 && *ID != 1982485 &&
			*ID != 1368083)
			return;

		float* X = (float*)((int)This + 0xC);
		float* Y = (float*)((int)This + 0x1C);
		float* Z = (float*)((int)This + 0x2C);

		*X = -100;
		*Y = -100;
		*Z = -100;
	}
}

int smallest;

CharacterEntry* Other::doesCharacterExist(Character* chr)
{
	for (vector<CharacterEntry*>::iterator itr = characters.begin(); itr != characters.end(); ++itr)
	{
		if ((*itr)->m_character == chr)
			return *itr;
	}
	return NULL;
}

CharacterEntry::CharacterEntry(Character* chr, char* name)
{
	this->m_character = chr;
	this->m_name = name;
	this->m_freezeAnimations = false;
}

void Other::CharacterHook(void* This)
{
	Character* chr = (Character*)This;
	if (*(void**)chr == Offsets::AI_TypeInfo_Character && string(chr->m_name) != "")
	{
		CharacterEntry* entry = doesCharacterExist(chr);
		if (!entry)
		{
			characters.push_back(new CharacterEntry(chr, chr->m_name));
			Log::Write("New character: \"" + string(chr->m_name) + "\" 0x" + Log::convertToHexa(chr));
		}
	}

	for (vector<CharacterEntry*>::iterator itr = characters.begin(); itr != characters.end(); ++itr)
	{
		if (*(void**)((*itr)->m_character) != Offsets::AI_TypeInfo_Character)
		{
			Log::Write("Removing character");
			if ((*itr)->m_character == pChar_Player)
				pChar_Player = NULL;
			characters.erase(itr);
			return;
		}
	}
}

void Other::toggleAlienFreeze()
{
	if (!pChar_Alien)
		return;
	m_alienFrozen = !m_alienFrozen;
	pChar_Alien->m_active = !m_alienFrozen;
}

void Other::toggleInvisibility()
{
	if (!pChar_Player)
	{
		int smallest;
		Character* chr = NULL;
		if (characters.begin() != characters.end())
		{
			smallest = (*characters.begin())->m_character->ID;
			chr = (*characters.begin())->m_character;
		}
		else
			return;
		for (vector<CharacterEntry*>::iterator itr = characters.begin(); itr != characters.end(); ++itr)
		{
			Log::Write(to_string((*itr)->m_character->ID));
			if ((*itr)->m_character->ID < smallest)
			{
				Log::Write("Smaller ID found");
				smallest = (*itr)->m_character->ID;
				chr = (*itr)->m_character;
			}
		}
		pChar_Player = chr;
		Log::Write("pChar_Player: 0x" + Log::convertToHexa(pChar_Player));
	}
	if (!pChar_Player)
		return;
	m_invisible = !m_invisible;
	pChar_Player->m_hidden = m_invisible;
}

void Other::toggleActorFreeze(std::string name)
{
	for (vector<CharacterEntry*>::iterator itr = characters.begin(); itr != characters.end(); ++itr)
	{
		if ((*itr)->m_name == name)
		{
			(*itr)->m_character->m_active = !(*itr)->m_character->m_active;
		}
	}
}

void Other::toggleAnimationFreeze(std::string name)
{
	for (vector<CharacterEntry*>::iterator itr = characters.begin(); itr != characters.end(); ++itr)
	{
		if ((*itr)->m_name == name)
		{
			(*itr)->m_freezeAnimations = !(*itr)->m_freezeAnimations;
		}
	}
}

void Other::Freeze1Hook(void* This, void* EAX)
{
	Character* chr = (Character*)((int)This - 0x3F0);
	for (vector<CharacterEntry*>::iterator itr = characters.begin(); itr != characters.end(); ++itr)
	{
		if ((*itr)->m_character == chr)
		{
			if ((*itr)->m_freezeAnimations)
			{
				int* flag = (int*)((int)Offsets::AI_Freeze2 + 0x4);
				*flag = 1;
			}
			return;
		}
	}
}

void Other::toggleUI()
{
	m_hideUI = !m_hideUI;
	Log::Write("Hide UI: " + to_string(m_hideUI));
}

void Other::UIHook(void* This)
{
	bool* active = (bool*)((int)This + 0x28);
	if (m_hideUI)
		*active = false;
	else
		*active = true;
}
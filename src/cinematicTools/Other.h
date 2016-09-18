#pragma once
#include <Windows.h>
#include <vector>

class Character
{
public:
	BYTE Pad1[0x10];
	char m_name[0x10];
	BYTE Pad2[0x30];
	int ID;
	BYTE Pad3[0x20C];
	bool m_active;
	BYTE Pad4[0x79];
	bool m_hidden;
};

class CharacterEntry
{
public:
	CharacterEntry(Character*, char*);
	Character* m_character;
	char* m_name;
	bool m_freezeAnimations;
};

class Player
{
public:
	virtual void Function1();
	virtual void Function2();
	virtual void Function3();
	virtual void Function4();
	virtual void Function5();
	virtual void Function6();
	virtual void Function7();
	virtual void Function8();
	virtual void Function9();
	virtual void Function10();
	virtual void Function11();
	virtual void Function12();
	virtual void Function13();
	virtual void Function14();
	virtual void Function15();
	virtual void Function16();
	virtual void Function17();
	virtual void Function18();
	virtual void Function19();
	virtual void Function20();
	virtual void Function21();
	virtual void Function22();
	virtual void Function23();
	virtual void DisableHUD();
};

class Other
{
public:
	void PlayerHook(void* This);
	Player* pPlayer;

	void CharacterHook(void* This);
	Character* pChar_Player;
	Character* pChar_Alien;

	void Freeze1Hook(void* This, void* EAX);

	void helmetHook(void* This);
	void toggleHelmet();
	
	void toggleInvisibility();
	void toggleAlienFreeze();

	void UIHook(void*);
	void toggleUI();

	void toggleActorFreeze(std::string name);
	void toggleAnimationFreeze(std::string name);

	bool* isHelmetRemoved() { return &m_helmetRemoved; }
	bool* isInvisible() { return &m_invisible; }
	bool* isAlienFrozen() { return &m_alienFrozen; }
	bool* isUIHidden() { return &m_hideUI; }

	std::vector<CharacterEntry*> characters;

	void lowerIndex();
	void biggerIndex();

private:
	bool m_helmetRemoved;
	bool m_invisible;
	bool m_alienFrozen;

	bool m_hideUI;

	CharacterEntry* doesCharacterExist(Character*);
};
#include "Menu.h"
#include <iomanip>
#include <stdlib.h>
#include <iostream>
#include <conio.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "Main.h"
#include "Tools\Log.h"
#include "Camera.h"
#include "Offsets.h"
#include "Renderer.h"
#include <windows.h>

using namespace std;

int Menu::selectedMenu;
int Menu::selectedIndex;
std::vector<MenuList*> Menu::menus;

bool Menu::m_initialized;
bool Menu::m_drawMenu;

Dx11Renderer m_pDx11Renderer;

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
	std::ostringstream out;
	out << std::setprecision(n) << a_value;
	return out.str();
}

float resolutionMultiplier_W = 1;
float resolutionMultiplier_H = 1;

float* selectedValue;
double* selectedDoubleValue;
float increaseAmount;
bool isDouble;

string inputString = "";
bool inputActive;

void(__thiscall Camera::* pFunc)() = &Camera::toggleCamera;
void* pPtr = (void*&)pFunc;

void toggleCamera()
{
	Main::m_camera.toggleCamera();
}

void togglePostProcess()
{
	Main::m_postProcess.Toggle();
}

void disableHUD()
{
	Main::m_other.toggleUI();
}

void removeHelmet()
{
	Main::m_other.toggleHelmet();
}

void toggleInvisibility()
{
	Main::m_other.toggleInvisibility();
}

void toggleAlienFreeze()
{
	Main::m_other.toggleAlienFreeze();
}

void toggleCharacterFreeze()
{
	string name = Main::m_menu.getActorName();
	Main::m_other.toggleActorFreeze(name);
}

void toggleAnimationFreeze()
{
	string name = Main::m_menu.getActorName();
	Main::m_other.toggleAnimationFreeze(name);
}

void FreezeGame()
{
	bool* freeze = (bool*)Offsets::AI_FreezeGame;
	*freeze = !*freeze;
}

void onRecordCamera()
{
	if (Main::m_camera.m_replayController.isRecording())
	{
		Main::m_camera.m_replayController.stopRecording();
	}
	else
	{
		Main::m_camera.m_replayController.startRecording();
	}
}

void onPlaybackCamera()
{
	if (Main::m_camera.m_replayController.isPlaying())
	{
		Main::m_camera.m_replayController.stopPlayback();
	}
	else
	{
		Main::m_camera.m_replayController.startPlayback();
	}
}


MenuItem::MenuItem()
{
}

void MenuItem::Draw(bool selected, XMFLOAT2 position, int tab)
{
	if (selected)
		m_pDx11Renderer._RenderText(position.x*resolutionMultiplier_W, position.y*resolutionMultiplier_H, 0xFF1010FF, 14.0f*resolutionMultiplier_W, FW1_RESTORESTATE, this->label);
	else
		m_pDx11Renderer._RenderText(position.x*resolutionMultiplier_W, position.y*resolutionMultiplier_H, 0xFFFFFFFF, 14.0f*resolutionMultiplier_W, FW1_RESTORESTATE, this->label);
}

void MenuItem::Click()
{

}

SubMenuItem::SubMenuItem(char* name, int index)
{
	this->label = name;
	this->index = index;
}

void SubMenuItem::Click()
{
	Main::m_menu.setSelectedMenu(this->index);
}

ToggleableItem::ToggleableItem(char* name, void* function, std::function<bool()> valuePred)
{
	this->label = name;
	this->Function = function;
	this->m_valuePredicate = valuePred;
}

ToggleableItem::ToggleableItem(char* name, void* function, bool* value)
	: ToggleableItem(name, function, [value]{ return *value; })
{}

void ToggleableItem::Draw(bool selected, XMFLOAT2 position, int tab)
{
	if (selected)
		m_pDx11Renderer._RenderText(position.x*resolutionMultiplier_W, position.y*resolutionMultiplier_H, 0xFF1010FF, 14.0f*resolutionMultiplier_W, FW1_RESTORESTATE, this->label);
	else
		m_pDx11Renderer._RenderText(position.x*resolutionMultiplier_W, position.y*resolutionMultiplier_H, 0xFFFFFFFF, 14.0f*resolutionMultiplier_W, FW1_RESTORESTATE, this->label);
	if (m_valuePredicate())
		m_pDx11Renderer._RenderText(position.x*resolutionMultiplier_W + tab*resolutionMultiplier_W, position.y*resolutionMultiplier_H, 0xFFFFFFFF, 14.0f*resolutionMultiplier_W, FW1_RESTORESTATE, "[X]");
	else
		m_pDx11Renderer._RenderText(position.x*resolutionMultiplier_W + tab*resolutionMultiplier_W, position.y*resolutionMultiplier_H, 0xFFFFFFFF, 14.0f*resolutionMultiplier_W, FW1_RESTORESTATE, "[  ]");
}

void ToggleableItem::Click()
{
	reinterpret_cast<void(*)()> (this->Function)();
}

ButtonItem::ButtonItem(char* name, void* function)
{
	this->label = name;
	this->Function = function;
}

void ButtonItem::Click()
{
	reinterpret_cast<void(*)()> (this->Function)();
}

SeparatorItem::SeparatorItem(char* separator)
{
	this->label = separator;
}

void SeparatorItem::Draw(bool selected, XMFLOAT2 position, int tab)
{
	//m_pDx11Renderer.RenderText(position.x, position.y, 0xFFFFFFFF, this->label);
	m_pDx11Renderer.DrawLine((position.x + 40) * resolutionMultiplier_W, (position.y + 10) * resolutionMultiplier_H, (position.x + 150) * resolutionMultiplier_W, (position.y + 10) * resolutionMultiplier_H, 0xFFC0C0C0);
}

DoubleItem::DoubleItem(char* name, double* value, double increase)
{
	this->label = name;
	this->m_value = value;
	this->m_increaseAmount = increase;
	this->m_input = false;
}

void DoubleItem::Click()
{
	m_input = !m_input;
	if (inputActive && !m_input)
	{
		DWORD dwVirtualProtectBackup;
		VirtualProtect(m_value, sizeof(*m_value), PAGE_READWRITE, &dwVirtualProtectBackup);
		char* e;
		errno = 0;
		double val = strtod(inputString.c_str(), &e);
		if (*e != '\0' || errno != 0)
			Log::WriteError("Invalid input");
		else
			*m_value = val;
		inputString = "";
		VirtualProtect(m_value, sizeof(*m_value), PAGE_READWRITE, &dwVirtualProtectBackup);
	}
	inputActive = m_input;
}

void DoubleItem::Draw(bool selected, XMFLOAT2 position, int tab)
{
	string finalLabel = to_string_with_precision(this->label, 5) + " " + to_string(*this->m_value);
	if (selected)
	{
		m_pDx11Renderer._RenderText(position.x*resolutionMultiplier_W, position.y*resolutionMultiplier_H, 0xFF1010FF, 14.0f*resolutionMultiplier_W, FW1_RESTORESTATE, this->label);
		if (!m_input)
			m_pDx11Renderer._RenderText((position.x + tab)*resolutionMultiplier_W, position.y*resolutionMultiplier_H, 0xFFFFFFFF, 14.0f*resolutionMultiplier_W, FW1_RESTORESTATE, (char*)to_string_with_precision(*this->m_value, 5).c_str());
		else
		{
			string finalString = inputString + "_";
			m_pDx11Renderer._RenderText((position.x + tab)*resolutionMultiplier_W, position.y*resolutionMultiplier_H, 0xFFFFFFFF, 14.0f*resolutionMultiplier_W, FW1_RESTORESTATE, (char*)finalString.c_str());
		}
	}
	else
	{
		m_pDx11Renderer._RenderText(position.x*resolutionMultiplier_W, position.y*resolutionMultiplier_H, 0xFFFFFFFF, 14.0f*resolutionMultiplier_W, FW1_RESTORESTATE, this->label);
		m_pDx11Renderer._RenderText(position.x*resolutionMultiplier_W + tab*resolutionMultiplier_W, position.y*resolutionMultiplier_H, 0xFFFFFFFF, 14.0f*resolutionMultiplier_W, FW1_RESTORESTATE, (char*)to_string_with_precision(*this->m_value, 5).c_str());
	}
}

FloatItem::FloatItem(char* name, float* value, float increase)
{
	this->label = name;
	this->m_value = value;
	this->m_increaseAmount = increase;
	this->m_input = false;
}

void FloatItem::Click()
{
	m_input = !m_input;
	if (inputActive && !m_input)
	{
		char* e;
		errno = 0;
		double val = strtod(inputString.c_str(), &e);
		if (*e != '\0' || errno != 0)
			Log::WriteError("Invalid input");
		else
			*m_value = val;
		inputString = "";
	}
	inputActive = m_input;
}

void FloatItem::Draw(bool selected, XMFLOAT2 position, int tab)
{
	string finalLabel = string(this->label) + " " + to_string_with_precision(*this->m_value, 5);
	if (selected)
	{
		m_pDx11Renderer._RenderText(position.x*resolutionMultiplier_W, position.y*resolutionMultiplier_H, 0xFF1010FF, 14.0f*resolutionMultiplier_W, FW1_RESTORESTATE, this->label);
		if (!m_input)
			m_pDx11Renderer._RenderText((position.x + tab)*resolutionMultiplier_W, position.y*resolutionMultiplier_H, 0xFFFFFFFF, 14.0f*resolutionMultiplier_W, FW1_RESTORESTATE, (char*)to_string_with_precision(*this->m_value, 5).c_str());
		else
		{
			string finalString = inputString + "_";
			m_pDx11Renderer._RenderText((position.x + tab)*resolutionMultiplier_W, position.y*resolutionMultiplier_H, 0xFFFFFFFF, 14.0f*resolutionMultiplier_W, FW1_RESTORESTATE, (char*)finalString.c_str());
		}
	}
	else
	{
		m_pDx11Renderer._RenderText(position.x*resolutionMultiplier_W, position.y*resolutionMultiplier_H, 0xFFFFFFFF, 14.0f*resolutionMultiplier_W, FW1_RESTORESTATE, this->label);
		m_pDx11Renderer._RenderText(position.x*resolutionMultiplier_W + tab*resolutionMultiplier_W, position.y*resolutionMultiplier_H, 0xFFFFFFFF, 14.0f*resolutionMultiplier_W, FW1_RESTORESTATE, (char*)to_string_with_precision(*this->m_value, 5).c_str());
	}
}

FreezeAnimationMenuList::FreezeAnimationMenuList(char* name, int parent, int tab, XMFLOAT2 size)
{
	this->name = name;
	this->parentMenu = parent;
	this->tab = tab;
	this->size = size;
}

CharacterMenuList::CharacterMenuList(char* name, int parent, int tab, XMFLOAT2 size)
{
	this->name = name;
	this->parentMenu = parent;
	this->tab = tab;
	this->size = size;
}

void FreezeAnimationMenuList::Update()
{
	if (items.size() != Main::m_other.characters.size())
	{
		Sleep(100);
		if (items.size() > Main::m_other.characters.size())
			items.clear();
		for (vector<CharacterEntry*>::iterator itr = Main::m_other.characters.begin(); itr != Main::m_other.characters.end(); ++itr)
		{
			bool found = false;
			for each(MenuItem* item in this->items)
			{
				if (item->label == (*itr)->m_name)
				{
					found = true;
				}
			}
			if (!found)
			{
				items.push_back(new ToggleableItem((*itr)->m_name, &toggleAnimationFreeze, &(*itr)->m_freezeAnimations));
			}
		}
	}
}


void CharacterMenuList::Update()
{
	if (items.size() != Main::m_other.characters.size())
	{
		Sleep(100);
		if (items.size() > Main::m_other.characters.size())
			items.clear();
		for (vector<CharacterEntry*>::iterator itr = Main::m_other.characters.begin(); itr != Main::m_other.characters.end(); ++itr)
		{
			bool found = false;
			for each(MenuItem* item in this->items)
			{
				if (item->label == (*itr)->m_name)
				{
					found = true;
				}
			}
			if (!found)
			{
				items.push_back(new ToggleableItem((*itr)->m_name, &toggleCharacterFreeze, &(*itr)->m_character->m_active));
			}
		}
	}
}

MenuList::MenuList()
{
}

MenuList::MenuList(char* name, vector<MenuItem*> items, int parent, int tab, XMFLOAT2 size)
{
	this->name = name;
	this->items = items;
	this->parentMenu = parent;
	this->tab = tab;
	this->size = size;
}

void MenuList::Exit()
{
	Main::m_menu.setSelectedMenu(this->parentMenu);
}

void MenuList::Click(int index)
{
	items.at(index)->Click();
}

void MenuList::Draw(int selectedIndex)
{
	m_pDx11Renderer._RenderText(110 * resolutionMultiplier_W, 110 * resolutionMultiplier_H, 0xFFFFFFFF, 20.0f*resolutionMultiplier_W, FW1_RESTORESTATE, this->name);
	int y = 140;
	for (int i = 0; i < items.size(); i++)
	{
		if (i == selectedIndex)
			items.at(i)->Draw(true, XMFLOAT2(110, y), tab);
		else
			items.at(i)->Draw(false, XMFLOAT2(110, y), tab);
		y += 20;
	}
}

void Menu::setSelectedMenu(int index)
{
	selectedIndex = 0;
	selectedMenu = index;
	if (menus[selectedMenu]->getItem(selectedIndex)->GetType() == string("FloatItem"))
	{
		FloatItem* item = (FloatItem*)menus[selectedMenu]->getItem(selectedIndex);
		selectedValue = item->getValue();
		increaseAmount = item->getIncreaseAmount();
	}
}

void Menu::Init(IDXGISwapChain* SwapChain)
{
	ID3D11DeviceContext* pContext;
	ID3D11Device* pDevice;

	HRESULT result = SwapChain->GetDevice(__uuidof(pDevice), (void**)&pDevice);
	if (FAILED(result))
	{
		Log::WriteError("Couldn't get ID3D11Device");
		return;
	}
	Sleep(100);
	pDevice->GetImmediateContext(&pContext);
	if (!pContext)
	{
		Log::WriteError("Couldn't get ID3D11DeviceContext");
		return;
	}

	Init(pDevice, pContext);
}

void Menu::Init(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	Log::Write("Initializing menu...");

	bool OK = m_pDx11Renderer.InitializeRenderClass(pDevice, pContext, 14.0f*resolutionMultiplier_W, "Arial", FW1_RESTORESTATE);

	if (!OK)
	{
		Log::WriteError("Could not initialize rendering class");
	}

	std::vector<MenuItem*> MainMenu;
	MainMenu.push_back(new ToggleableItem("Toggle freecamera", &toggleCamera, Main::m_camera.isEnabled()));
	MainMenu.push_back(new SubMenuItem("Freecamera options", 1));
	MainMenu.push_back(new SubMenuItem("Post processing", 2));
	MainMenu.push_back(new SeparatorItem("----"));
	MainMenu.push_back(new FloatItem("Field of view", &Main::m_camera.m_camera.m_fov, 0.2f));
	MainMenu.push_back(new DoubleItem("Timescale", (double*)Offsets::AI_Time, 0.01));
	MainMenu.push_back(new ToggleableItem("Hide HUD", &disableHUD, Main::m_other.isUIHidden()));
	MainMenu.push_back(new ToggleableItem("Remove helmet", &removeHelmet, Main::m_other.isHelmetRemoved()));
	MainMenu.push_back(new ToggleableItem("Invisibility", &toggleInvisibility, Main::m_other.isInvisible()));
	//MainMenu.push_back(new ToggleableItem("Freeze Alien", &toggleAlienFreeze, Main::m_other.isAlienFrozen()));
	MainMenu.push_back(new SubMenuItem("Freeze actors", 3));
	MainMenu.push_back(new SubMenuItem("Freeze animations", 4));
	MainMenu.push_back(new ToggleableItem("Freeze game", &FreezeGame, (bool*)Offsets::AI_FreezeGame));
	MainMenu.push_back(new SeparatorItem("----"));
	MainMenu.push_back(new ToggleableItem("Record camera", &onRecordCamera, [=]{ return Main::m_camera.m_replayController.isRecording(); }));
	MainMenu.push_back(new ToggleableItem("Playback camera", &onPlaybackCamera, [=]{ return Main::m_camera.m_replayController.isPlaying(); }));

	std::vector<MenuItem*> freeCamOptions;
	freeCamOptions.push_back(new FloatItem("Movement speed", &Main::m_camera.m_camera.m_speed, 0.01f));
	freeCamOptions.push_back(new FloatItem("Rotation speed", &Main::m_camera.m_camera.m_rotationSpeed, 0.01f));
	freeCamOptions.push_back(new FloatItem("Roll speed", &Main::m_camera.m_camera.m_rollSpeed, 0.01f));
	freeCamOptions.push_back(new FloatItem("Boost multiplier", &Main::m_camera.m_camera.m_boostMultiplier, 0.01f));

	std::vector<MenuItem*> postProcess;
	postProcess.push_back(new ToggleableItem("Enable", &togglePostProcess, Main::m_postProcess.isEnabled()));
	postProcess.push_back(new FloatItem("Contrast", &Main::m_postProcess.m_settings.m_contrast, 0.0001f));
	postProcess.push_back(new FloatItem("Brightness", &Main::m_postProcess.m_settings.m_brightness, 0.01f));
	postProcess.push_back(new FloatItem("Saturation", &Main::m_postProcess.m_settings.m_saturation, 0.01f));
	postProcess.push_back(new FloatItem("Red tint", &Main::m_postProcess.m_settings.m_tint.x, 0.01f));
	postProcess.push_back(new FloatItem("Green tint", &Main::m_postProcess.m_settings.m_tint.y, 0.01f));
	postProcess.push_back(new FloatItem("Blue tint", &Main::m_postProcess.m_settings.m_tint.z, 0.01f));
	postProcess.push_back(new SeparatorItem("Separator"));
	postProcess.push_back(new FloatItem("DoF Strength", &Main::m_postProcess.m_settings.m_dofStrength, 0.001f));
	postProcess.push_back(new FloatItem("DoF Focus", &Main::m_postProcess.m_settings.m_dofFocusDistance, 0.02f));
	postProcess.push_back(new FloatItem("DoF Scale", &Main::m_postProcess.m_settings.m_dofScale, 0.02f));
	postProcess.push_back(new SeparatorItem("Separator"));
	postProcess.push_back(new FloatItem("Distort Factor", &Main::m_postProcess.m_settings.m_radialDistortFactor, 0.01f));
	postProcess.push_back(new FloatItem("Distort Constraint", &Main::m_postProcess.m_settings.m_radialDistortConstraint, 0.01f));
	postProcess.push_back(new FloatItem("Distort Scalar", &Main::m_postProcess.m_settings.m_radialDistortScalar, 0.01f));
	postProcess.push_back(new SeparatorItem("Separator"));
	postProcess.push_back(new FloatItem("Film Grain #1", &Main::m_postProcess.m_settings.m_filmGrain, 0.01f));
	postProcess.push_back(new FloatItem("Film Grain #1", &Main::m_postProcess.m_settings.m_filmGrain2, 0.01f));
	postProcess.push_back(new FloatItem("Film Grain Lights", &Main::m_postProcess.m_settings.m_filmGrainLights, 0.01f));
	postProcess.push_back(new SeparatorItem("Separator"));
	postProcess.push_back(new FloatItem("Bloom", &Main::m_postProcess.m_settings.m_bloom, 0.01f));
	postProcess.push_back(new FloatItem("Vignette", &Main::m_postProcess.m_settings.m_vignetteStrength, 0.01f));
	postProcess.push_back(new FloatItem("Chromatic", &Main::m_postProcess.m_settings.m_chromaticStrength, 0.002f));
	postProcess.push_back(new FloatItem("Desharp", &Main::m_postProcess.m_settings.m_sharpness, 0.01f));
	postProcess.push_back(new FloatItem("Lens effect", &Main::m_postProcess.m_settings.m_lenseThing, 0.01f));

	menus.push_back(new MenuList("MAIN MENU", MainMenu, 0, 140, XMFLOAT2(100, 18 * MainMenu.size())));
	menus.push_back(new MenuList("Freecamera Options", freeCamOptions, 0, 130, XMFLOAT2(100, 180)));
	menus.push_back(new MenuList("Post processing", postProcess, 0, 130, XMFLOAT2(100, 460)));
	menus.push_back(new CharacterMenuList("Freeze character", 0, 140, XMFLOAT2(100, 400)));
	menus.push_back(new FreezeAnimationMenuList("Freeze animations", 0, 140, XMFLOAT2(100, 400)));

	Log::WriteOK("Menu initialized");
	m_initialized = true;
	m_drawMenu = true;
}

void Menu::Draw()
{
	if (m_drawMenu && m_initialized)
	{
		if (Offsets::AI_Resolution)
		{
			resolutionMultiplier_W = (float)(*(int*)Offsets::AI_Resolution) / 1920;
			resolutionMultiplier_H = (float)(*(int*)((int)Offsets::AI_Resolution+0x4)) / 1080;
			if (resolutionMultiplier_W < 1 || resolutionMultiplier_H < 1)
			{
				resolutionMultiplier_H = 1;
				resolutionMultiplier_W = 1;
			}
		}
		XMFLOAT2 dimension = menus[selectedMenu]->getDimension();
		m_pDx11Renderer.DrawBox(100 * resolutionMultiplier_W, 100 * resolutionMultiplier_H, (100 + dimension.x) * resolutionMultiplier_W, (100 + dimension.y) * resolutionMultiplier_H, 0xA0000000, 0xFFFFFFFF);
		menus[selectedMenu]->Draw(selectedIndex);
	}
}

void Menu::Update()
{
	while (true)
	{
		if (!m_initialized) continue;

		CharacterMenuList* chrMenu = (CharacterMenuList*)menus.at(3);
		FreezeAnimationMenuList* animMenu = (FreezeAnimationMenuList*)menus.at(4);
		chrMenu->Update();
		animMenu->Update();

		//if (!*Offsets::AI_Focus) continue;

		if (GetAsyncKeyState(VK_INSERT) & 0x8000)
		{
			if (inputActive)
			{
				inputActive = false;
				menus[selectedMenu]->Click(selectedIndex);
			}
			m_drawMenu = !m_drawMenu;
			Sleep(200);
		}

		if (!m_drawMenu) continue;

		const WORD gamepadButtons = Main::m_camera.getGamepadState().Gamepad.wButtons;

		if ((GetAsyncKeyState(VK_RETURN) & 0x8000) || (GetAsyncKeyState(VK_RSHIFT) & 0x8000) || (gamepadButtons & XINPUT_GAMEPAD_A))
		{
			menus[selectedMenu]->Click(selectedIndex);
			Sleep(200);
		}
		if ((GetAsyncKeyState(VK_BACK) & 0x8000) || (gamepadButtons & XINPUT_GAMEPAD_B))
		{
			if (inputActive)
			{
				if (inputString.size() > 0)
					inputString = inputString.substr(0, inputString.size() - 1);
				Sleep(100);
				continue;
			}
			menus[selectedMenu]->Exit();
			Sleep(200);
		}

		if (inputActive)
		{
			if (GetAsyncKeyState(0x30) & 0x8000 || GetAsyncKeyState(VK_NUMPAD0) & 0x8000)
			{
				inputString += "0";
				Sleep(150);
			}
			if (GetAsyncKeyState(0x31) & 0x8000 || GetAsyncKeyState(VK_NUMPAD1) & 0x8000)
			{
				inputString += "1";
				Sleep(150);
			}
			if (GetAsyncKeyState(0x32) & 0x8000 || GetAsyncKeyState(VK_NUMPAD2) & 0x8000)
			{
				inputString += "2";
				Sleep(150);
			}
			if (GetAsyncKeyState(0x33) & 0x8000 || GetAsyncKeyState(VK_NUMPAD3) & 0x8000)
			{
				inputString += "3";
				Sleep(150);
			}
			if (GetAsyncKeyState(0x34) & 0x8000 || GetAsyncKeyState(VK_NUMPAD4) & 0x8000)
			{
				inputString += "4";
				Sleep(150);
			}
			if (GetAsyncKeyState(0x35) & 0x8000 || GetAsyncKeyState(VK_NUMPAD5) & 0x8000)
			{
				inputString += "5";
				Sleep(150);
			}
			if (GetAsyncKeyState(0x36) & 0x8000 || GetAsyncKeyState(VK_NUMPAD6) & 0x8000)
			{
				inputString += "6";
				Sleep(150);
			}
			if (GetAsyncKeyState(0x37) & 0x8000 || GetAsyncKeyState(VK_NUMPAD7) & 0x8000)
			{
				inputString += "7";
				Sleep(150);
			}
			if (GetAsyncKeyState(0x38) & 0x8000 || GetAsyncKeyState(VK_NUMPAD8) & 0x8000)
			{
				inputString += "8";
				Sleep(150);
			}
			if (GetAsyncKeyState(0x39) & 0x8000 || GetAsyncKeyState(VK_NUMPAD9) & 0x8000)
			{
				inputString += "9";
				Sleep(150);
			}
			if (GetAsyncKeyState(0xBE) & 0x8000 || GetAsyncKeyState(0xBC) & 0x8000)
			{
				inputString += ".";
				Sleep(150);
			}
			continue;
		}

		if ((GetAsyncKeyState(VK_DOWN) & 0x8000) || (gamepadButtons & XINPUT_GAMEPAD_DPAD_DOWN))
		{
		moveDown:
			selectedIndex += 1;
			if (selectedIndex == menus[selectedMenu]->getSize())
				selectedIndex = 0;

			if (menus[selectedMenu]->getItem(selectedIndex)->GetType() == string("FloatItem"))
			{
				FloatItem* item = (FloatItem*)menus[selectedMenu]->getItem(selectedIndex);
				selectedValue = item->getValue();
				increaseAmount = item->getIncreaseAmount();
				isDouble = false;
			}
			else if (menus[selectedMenu]->getItem(selectedIndex)->GetType() == string("DoubleItem"))
			{
				DoubleItem* item = (DoubleItem*)menus[selectedMenu]->getItem(selectedIndex);
				selectedDoubleValue = item->getValue();
				increaseAmount = item->getIncreaseAmount();
				isDouble = true;
			}
			else if (menus[selectedMenu]->getItem(selectedIndex)->GetType() == string("SeparatorItem"))
				goto moveDown;

			Sleep(150);
		}
		if ((GetAsyncKeyState(VK_UP) & 0x8000) || (gamepadButtons & XINPUT_GAMEPAD_DPAD_UP))
		{
		moveUp:
			selectedIndex -= 1;
			if (selectedIndex < 0)
				selectedIndex = menus[selectedMenu]->getSize() - 1;

			if (menus[selectedMenu]->getItem(selectedIndex)->GetType() == string("FloatItem"))
			{
				FloatItem* item = (FloatItem*)menus[selectedMenu]->getItem(selectedIndex);
				selectedValue = item->getValue();
				increaseAmount = item->getIncreaseAmount();
				isDouble = false;
			}
			else if (menus[selectedMenu]->getItem(selectedIndex)->GetType() == string("DoubleItem"))
			{
				DoubleItem* item = (DoubleItem*)menus[selectedMenu]->getItem(selectedIndex);
				selectedDoubleValue = item->getValue();
				increaseAmount = item->getIncreaseAmount();
				isDouble = true;
			}
			else if (menus[selectedMenu]->getItem(selectedIndex)->GetType() == string("SeparatorItem"))
				goto moveUp;
			else
				selectedValue = NULL;

			Sleep(150);
		}
		if ((GetAsyncKeyState(VK_ADD) & 0x8000) || (GetAsyncKeyState(VK_RIGHT) & 0x8000) || (gamepadButtons & XINPUT_GAMEPAD_DPAD_RIGHT))
		{
			if (selectedValue && !isDouble)
			{
				*selectedValue = *selectedValue + increaseAmount;
			}
			else if (selectedDoubleValue && isDouble)
			{
				DWORD dwVirtualProtectBackup;
				VirtualProtect(selectedDoubleValue, sizeof(*selectedDoubleValue), PAGE_READWRITE, &dwVirtualProtectBackup);
				*selectedDoubleValue = *selectedDoubleValue + increaseAmount;
				VirtualProtect(selectedDoubleValue, sizeof(*selectedDoubleValue), dwVirtualProtectBackup, NULL);
			}
			Sleep(10);
		}
		if ((GetAsyncKeyState(VK_SUBTRACT) & 0x8000) || (GetAsyncKeyState(VK_LEFT) & 0x8000) || (gamepadButtons & XINPUT_GAMEPAD_DPAD_LEFT))
		{
			if (selectedValue && !isDouble)
				*selectedValue = *selectedValue - increaseAmount;
			else if (selectedDoubleValue && isDouble)
			{
				DWORD dwVirtualProtectBackup;
				VirtualProtect(selectedDoubleValue, sizeof(*selectedDoubleValue), PAGE_READWRITE, &dwVirtualProtectBackup);
				*selectedDoubleValue = *selectedDoubleValue - increaseAmount;
				VirtualProtect(selectedDoubleValue, sizeof(*selectedDoubleValue), dwVirtualProtectBackup, NULL);
			}
			Sleep(10);
		}
		Sleep(1);
	}
}
#pragma once

#include <string>
#include <fstream>
#include <windows.h>
#include <sstream>
#include <vector>
#include <functional>
#include <d3d11.h>
#include <DirectXMath.h>

using namespace std;
using namespace DirectX;

class MenuItem
{
public:
	MenuItem();
	virtual void Draw(bool selected, XMFLOAT2 position, int tab);
	virtual void Click();
	virtual string GetType() { return "MenuItem"; }
	char* label;
};

class SubMenuItem : public MenuItem
{
public:
	SubMenuItem(char* name, int index);
	void Click();
	string GetType() { return "SubMenuItem"; }

private:
	int index;
};

class SeparatorItem : public MenuItem
{
public:
	SeparatorItem(char* separator);
	void Draw(bool selected, XMFLOAT2 position, int tab);
	string GetType() { return "SeparatorItem"; }
};

class FloatItem : public MenuItem
{
public:
	FloatItem(char* name, float* value, float increaseAmount);
	void Click();
	void Draw(bool selected, XMFLOAT2 position, int tab);
	string GetType() { return "FloatItem"; };

	float* getValue() { return m_value; }
	float getIncreaseAmount() { return m_increaseAmount; }

private:
	float* m_value;
	float m_increaseAmount;
	bool m_input;
};

class DoubleItem : public MenuItem
{
public:
	DoubleItem(char* name, double* value, double increaseAmount);
	void Click();
	void Draw(bool selected, XMFLOAT2 position, int tab);
	string GetType() { return "DoubleItem"; }

	double* getValue() { return m_value; }
	double getIncreaseAmount() { return m_increaseAmount; }

private:
	double* m_value;
	double m_increaseAmount;
	bool m_input;
};

class ToggleableItem : public MenuItem
{
public:
	ToggleableItem(char* name, void* Function, std::function<bool()> valuePred);
	ToggleableItem(char* name, void* Function, bool* value);
	void Draw(bool selected, XMFLOAT2 position, int tab);
	void Click();
	string GetType() { return "ToggeleableItem"; };

private:
	std::function<bool()> m_valuePredicate;
	//bool* m_toggled;
	void* Function;
};

class ButtonItem : public MenuItem
{
public:
	ButtonItem(char* name, void* function);
	void Click();
	string GetType() { return "ButtonItem"; };

private:
	void* Function;
};

class MenuList
{
public:
	MenuList();
	MenuList(char* name, vector<MenuItem*> items, int parent, int tab, XMFLOAT2 size);
	virtual void Draw(int index);
	void Click(int index);
	void Exit();

public:
	char* name;
	vector<MenuItem*> items;
	int parentMenu;
	int tab;
	XMFLOAT2 size;

public:
	int getSize()					{ return items.size(); }
	MenuItem* getItem(int index)	{ return items.at(index); }
	XMFLOAT2 getDimension()			{ return size; }

};

class CharacterMenuList : public MenuList
{
public:
	CharacterMenuList(char* name, int parent, int tab, XMFLOAT2 size);
	void Update();
	string getActorName(int index) { return string(this->items.at(index)->label); }
};

class FreezeAnimationMenuList : public MenuList
{
public:
	FreezeAnimationMenuList(char* name, int parent, int tab, XMFLOAT2 size);
	void Update();
	string getActorName(int index) { return string(this->items.at(index)->label); }
};

class Menu
{
public:
	static void Init(IDXGISwapChain* swapChain);
	static void Init(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	static void Draw();

	static void Update();

	static void setSelectedMenu(int index);
	static int getSelectedIndex() { return selectedIndex; }
	static string getActorName() { CharacterMenuList* chrMenu = (CharacterMenuList*)menus.at(3); return chrMenu->getActorName(selectedIndex); }

private:
	static int selectedMenu;
	static int selectedIndex;
	static std::vector<MenuList*> menus;

	static bool m_initialized;
	static bool m_drawMenu;

};
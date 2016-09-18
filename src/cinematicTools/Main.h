#pragma once
#include "Camera.h"
#include "Menu.h"
#include "PostProcess.h"
#include "Other.h"

class Main
{
public:
	static void Init();
	static Camera m_camera;
	static Menu m_menu;
	static int color;
	static XMFLOAT2 position;
	static PostProcess m_postProcess;
	static Other m_other;

private:
	static void Update();

};
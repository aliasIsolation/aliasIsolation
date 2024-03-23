#pragma once
#include <string>

struct Settings
{
	float sharpening = 0.6f;
	bool sharpeningEnabled = true;
	float chromaticAberration = 0.5f;
	bool chromaticAberrationEnabled = true;
};

void setSettingsFilePath(const char*);
bool loadSettings(Settings*);
bool saveSettings(const Settings&);

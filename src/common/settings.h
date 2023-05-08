#pragma once
#include <string>

struct Settings
{
	float sharpening = 0.6f;
	float chromaticAberration = 0.5f;
};

void setSettingsFilePath(const char*);
bool loadSettings(Settings*);
void saveSettings(const Settings&);

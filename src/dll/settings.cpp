#include "settings.h"

#include <string>

#include "windows.h"

namespace {
	std::string g_settingsFilePath;
}

void setSettingsFilePath(const char* path)
{
	g_settingsFilePath = path;
}

bool loadSettings(Settings* settingsOut)
{
	FILE* f;
	errno_t err = fopen_s(&f, g_settingsFilePath.c_str(), "r");
	if (err != 0) {
		return false;
	}

	if (f) {
		int scanCount = fscanf_s(f, "sharpening = %f\n", &settingsOut->sharpening);
		if (scanCount == 0 || scanCount == EOF) {
			return false;
		}

		scanCount = fscanf_s(f, "chromaticAberration = %f\n", &settingsOut->chromaticAberration);
		if (scanCount == 0 || scanCount == EOF) {
			return false;
		}

		int ret = fclose(f);
		if (ret != 0 || ret == EOF) {
			return false;
		}

		settingsOut->sharpeningEnabled = (settingsOut->sharpening > 0.0f);
		settingsOut->chromaticAberrationEnabled = (settingsOut->chromaticAberration > 0.0f);

		return true;
	}

	return false;
}

bool saveSettings(const Settings& settings)
{
	FILE* f;
	errno_t err = fopen_s(&f, g_settingsFilePath.c_str(), "w");
	if (err != 0) {
		return false;
	}

	if (f) {
		float saveValue = 0.0f;
		if (settings.sharpeningEnabled) {
			saveValue = settings.sharpening;
		}
		int writeCount = fprintf_s(f, "sharpening = %f\n", saveValue);
		if (writeCount == -1) {
			return false;
		}

		saveValue = 0.0f;
		if (settings.chromaticAberrationEnabled) {
			saveValue = settings.chromaticAberration;
		}
		writeCount = fprintf_s(f, "chromaticAberration = %f\n", saveValue);
		if (writeCount == -1) {
			return false;
		}

		int ret = fclose(f);
		if (ret != 0 || ret == EOF) {
			return false;
		}

		return true;
	}

	return false;
}

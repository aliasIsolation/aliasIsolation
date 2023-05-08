#include "settings.h"
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

    int scanCount = fscanf_s(f, "sharpening = %f\n", &settingsOut->sharpening);
    if (scanCount == 0 || scanCount == EOF) {
      return false;
    }

	scanCount = fscanf_s(f, "chromaticAberration = %f\n", &settingsOut->chromaticAberration);
    if (scanCount == 0 || scanCount == EOF) {
      return false;
    }

	fclose(f);

	return true;
}

void saveSettings(const Settings& settings)
{
	for (int retry = 0; retry < 10; ++retry, Sleep(10))
	{
		FILE* f;
		errno_t err = fopen_s(&f, g_settingsFilePath.c_str(), "w");
		if (err != 0) {
			continue;
		}

		fprintf_s(f, "sharpening = %f\n", settings.sharpening);
		fprintf_s(f, "chromaticAberration = %f\n", settings.chromaticAberration);
		fclose(f);
		break;
	}
}

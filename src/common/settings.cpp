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
	FILE* f = fopen(g_settingsFilePath.c_str(), "r");
	if (!f) {
		return false;
	}

	char buf[1024];
	fgets(buf, sizeof(buf)-1, f);
	int bufLen = strlen(buf);
	if (bufLen > 1 && buf[bufLen-1] == '\n') buf[bufLen-1] = '\0';
	settingsOut->aiExePath = buf;

	fscanf(f, "sharpening = %f\n", &settingsOut->sharpening);
	fscanf(f, "chromaticAberration = %f\n", &settingsOut->chromaticAberration);
	fclose(f);

	return true;
}

void saveSettings(const Settings& settings)
{
	for (int retry = 0; retry < 10; ++retry, Sleep(10))
	{
		FILE* f = fopen(g_settingsFilePath.c_str(), "w");
		if (!f) {
			continue;
		}

		fprintf(f, "%s\n", settings.aiExePath.c_str(), f);
		fprintf(f, "sharpening = %f\n", settings.sharpening);
		fprintf(f, "chromaticAberration = %f\n", settings.chromaticAberration);
		fclose(f);
		break;
	}
}

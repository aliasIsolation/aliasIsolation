#include "dllParams.h"
#include <windows.h>
#include <cstdio>
#include <string>


namespace
{
	std::string getSharedParamsFilePath()
	{
		char tempPath[MAX_PATH];
		GetTempPathA(MAX_PATH, tempPath);
		return std::string(tempPath) + "\\aliasIsolationSharedParams.bin";
	}
}

SharedDllParams getSharedDllParams()
{
	char tempPath[MAX_PATH];
	FILE* f = fopen(getSharedParamsFilePath().c_str(), "rb");
	if (!f) DebugBreak();

	SharedDllParams params;
	size_t rb = fread(&params, 1, sizeof(params), f);
	if (rb != sizeof(params)) DebugBreak();

	fclose(f);
	return params;
}

void setSharedDllParams(const SharedDllParams& params)
{
	char tempPath[MAX_PATH];
	FILE* f = fopen(getSharedParamsFilePath().c_str(), "wb");
	if (!f) DebugBreak();

	size_t wb = fwrite(&params, 1, sizeof(params), f);
	if (wb != sizeof(params)) DebugBreak();

	fclose(f);
}

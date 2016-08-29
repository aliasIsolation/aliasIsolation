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
	if (!f) abort();

	SharedDllParams params;
	size_t rb = fread(&params, 1, sizeof(params), f);
	if (rb != sizeof(params)) abort();

	fclose(f);
	return params;
}

void setSharedDllParams(const SharedDllParams& params)
{
	char tempPath[MAX_PATH];
	FILE* f = fopen(getSharedParamsFilePath().c_str(), "wb");
	if (!f) abort();

	size_t wb = fwrite(&params, 1, sizeof(params), f);
	if (wb != sizeof(params)) abort();

	fclose(f);
}

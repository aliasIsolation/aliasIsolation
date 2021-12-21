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
	SharedDllParams params;
	FILE* f = fopen(getSharedParamsFilePath().c_str(), "rb");

	if (!f)
	{
		// Fail silently if we cannot get the shared parameters file.
		//printf_s("[aliasIsolation::dllParams] Non-fatal Error - Failed to open shared params file.\n");
		return params;
	}

	size_t rb = fread(&params, 1, sizeof(params), f);
	if (rb != sizeof(params)) DebugBreak();

	fclose(f);
	return params;
}

void setSharedDllParams(const SharedDllParams& params)
{
	FILE* f = fopen(getSharedParamsFilePath().c_str(), "wb");
	
	if (!f)
	{
		// Fail silently if we cannot get the shared parameters file.
		printf_s("[aliasIsolation::dllParams] Non-fatal Error - Failed to open shared params file.\n");
		return;
	}

	size_t wb = fwrite(&params, 1, sizeof(params), f);
	if (wb != sizeof(params)) DebugBreak();

	fclose(f);
}

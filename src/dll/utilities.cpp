#include "utilities.h"

#include <Windows.h>
#include <filesystem>

std::string rootDir;


std::string getRootDirectory() {
	// Only fetch the root directory once.
	if (rootDir.empty())
	{
		// Store the path relative to the current DLL (really AI.exe since we're injected into it).
		rootDir = std::filesystem::current_path().string() + "\\mods\\aliasIsolation";

		// Have we got the root directory?
		if (!rootDir.empty())
		{
			// We have got the root directory.
			LOG_MSG("[aliasIsolation::utilities] Got root directory \"%s\".\n", rootDir.c_str());
		}
		else
		{
			// We failed to get the root directory, break execution here.
			LOG_MSG("[aliasIsolation::utilities] FATAL ERROR - Failed to get root directory!\n", "");
			DebugBreak();
		}
	}

	return rootDir;
}

std::string getSettingsFilePath() {
	WIN32_FIND_DATA findSettingsFileData;
	HANDLE hFind;

	// Build the path to the settings file.
	std::string path = getRootDirectory().append("\\settings.txt");

	// Scan the path for the settings file.
	hFind = FindFirstFileA(path.c_str(), &findSettingsFileData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		LOG_MSG("[aliasIsolation::utilities] Non-fatal Error - Failed to find settings file at \"%s\"!\n", path.c_str());
	}
	else
	{
		LOG_MSG("[aliasIsolation::utilities] Located settings file at \"%s\".\n", path.c_str());
	}

	return path;
}

std::string getDataFilePath(std::string file, bool critical) {
	WIN32_FIND_DATA findDataFileData;
	HANDLE hFind;

	// Build the path from the file name provided.
	std::string path = getRootDirectory().append("\\data\\" + file);

	// Scan the path for the specified data file.
	hFind = FindFirstFileA(path.c_str(), &findDataFileData);

	// We couldn't find the data file at the path provided.
	if (hFind == INVALID_HANDLE_VALUE)
	{
		// Is this a critical data file (i.e. a shader)?
		if (critical)
		{
			// Inform the user via a message box and throw a debug break to create a dump file.
			LOG_MSG("[aliasIsolation::utilities] FATAL ERROR - Failed to find critical data file at \"%s\"!\n", path.c_str());
			
			char buf[256];
			sprintf_s(buf, "FATAL ERROR - Failed to find critical data file \"%s\"!", path.c_str());
			MessageBoxA(NULL, buf, "Alias Isolation", NULL);
			
			DebugBreak();
		}
		else
		{
			// It isn't, continue execution anyway.
			LOG_MSG("[aliasIsolation::utilities] Non-fatal Error - Failed to find data file at \"%s\"!\n", path.c_str());
		}
	}
	else
	{
		LOG_MSG("[aliasIsolation::utilities] Located data file at \"%s\".\n", path.c_str());
	}
	
	return path;
}

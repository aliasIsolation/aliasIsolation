#pragma once
#include <string>
#include "common.h"


/// <summary>
/// Returns the path to the root directory for the mod, this is fetched via SharedDllParams (provided by the injector EXEs) or a call to the current_path function.
/// This function will only run the logic to fetch the root directory once, after that, it simply returns the value of rootDir.
/// <para>
/// N.B - If there are no SharedDllParams values (i.e. if the injector EXEs were not used), then this function assumes that the path
/// where the game data is located must be where this DLL (or rather, AI.EXE since Alias Isolation's DLL is injected into it) currently exists.
/// </para>
/// </summary>
/// <returns>The directory where Alias Isolation's root directory is located (as a string).</returns>
/// <remarks>
/// This function was added to maintain compatibility with the injector EXEs and to centralise this logic.
/// </remarks>
std::string getRootDirectory();

/// <summary>
/// Returns the fully qualified path to the settings.txt file.
/// <para>
/// Note: This function will verify the existence of the file and return the path using the relative path provided by <see cref="getRootDirectory"/>.
/// </para>
/// </summary>
/// <returns>The fully qualified path to the settings.txt file.</returns>
std::string getSettingsFilePath();

/// <summary>
/// Returns the fully qualified path to the passed data file.
/// Note: This function will verify the existence of a file and return the path using the path provided by <see cref="getRootDirectory"/>, this function always looks at the "data" directory inside the root directory.
/// <para>
/// The critical parameter is for specifying whether or not the passed data file is critical to the mod working, a data file that does not exist
/// with the critical variable set to true will cause Alias Isolation to trigger a crash.
/// </para>
/// </summary>
/// <param name="path">- The relative path to the data file.</param>
/// <param name="critical">- Whether or not the data file is critical to the mod working.</param>
/// <returns>The fully qualified path to the passed data file.</returns>
/// <remarks>
/// This function was added to maintain compatibility with the injector EXEs and to centralise the file existence verification logic.
/// </remarks>
std::string getDataFilePath(std::string path, bool critical);
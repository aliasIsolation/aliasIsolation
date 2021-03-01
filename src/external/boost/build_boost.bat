@echo off
setlocal enabledelayedexpansion

rem If we are provided with a configuration to build Boost with, then use that. (Boost builds both debug and release libraries otherwise, increasing build times slightly.)
if not "%1" == "" (
	set "BOOSTCONFIGURATION=%1"
)

rem If we are told to use x64, then we kick off an x64 build instead, otherwise, we build an x86 version.
if "%2" == "x64" (
	set "BOOSTADDRESSMODEL=64"
) else (
	set "BOOSTADDRESSMODEL=32"
)

rem Make sure Boost has been installed properly before continuing.
if not exist "bootstrap.bat" (
	goto ERR_NO_BOOTSTRAP_BAT
)

rem Compile the Boost B2 build system executable if it doesn't exist already.
if not exist "b2.exe" (
	rem Call Boost's bootstrap file which compiles their B2 build system executable.
	call bootstrap.bat
)

rem Ask B2 to compile boost with the configuration we need.
rem Release or debug, static runtime linking (a static library), multithreaded, 32 or 64 bit address model.
rem Boost's architecture is always x86, even for x64 builds, only the address model changes.
b2.exe %BOOSTCONFIGURATION% runtime-link=static threading=multi address-model=%BOOSTADDRESSMODEL% architecture=x86 --with-chrono --with-system --with-date_time
goto END

:ERR_NO_BOOTSTRAP_BAT
echo.
echo [Build failed] Failed to find bootstrap.bat. Have you copied Boost's source code to src/external/boost?

:END
echo.
echo Finished.
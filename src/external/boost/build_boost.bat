@echo off
setlocal enabledelayedexpansion

rem If we are provided with a configuration to build Boost with, then use that. (Boost builds both debug and release libraries otherwise, increasing build times slightly.)
if not "%1" == "" (
	set "BOOSTCONFIGURATION=%1"
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

b2.exe %BOOSTCONFIGURATION% runtime-link=static threading=multi address-model=32 architecture=x86 --with-chrono --with-system --with-date_time
goto END

:ERR_NO_BOOTSTRAP_BAT
echo.
echo [Build failed] Failed to find bootstrap.bat. Have you copied Boost's source code to src/external/boost?

:END
echo.
echo Finished.
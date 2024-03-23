@echo off
setlocal enabledelayedexpansion

rem If we are provided with a build configuration, then use that.
if "%1" == "release" (
	set "CONFIGURATION=release"
) else if "%1" == "debug" (
	set "CONFIGURATION=debug"
) else if "%1" == "" (
    rem If we are given no configuration at all, assume we wanted release.
    set "CONFIGURATION=release"
) else (
    goto ERR_INVALID_BUILD_TARGET
)

rem If we are provided with a build architecture, then use that.
if "%2" == "x64" (
    set "ARCHITECTURE=x64"
    set "TUNDRATARGET=win64-msvc-%CONFIGURATION%-default"
) else if "%2" == "x86" (
    set "ARCHITECTURE=x86"
    set "TUNDRATARGET=win32-msvc-%CONFIGURATION%-default"
) else if "%2" == "" (
    rem If we are given no architecture at all, assume we wanted x86.
    set "ARCHITECTURE=x86"
    set "TUNDRATARGET=win32-msvc-%CONFIGURATION%-default"
) else (
    goto ERR_INVALID_BUILD_ARCHITECTURE
)

rem Add extra options to pass to Tundra.
set "TUNDRAEXTRAOPTS=%3"

rem Compile the project with the chosen configuration.
call compile.cmd %CONFIGURATION% %ARCHITECTURE% %TUNDRAEXTRAOPTS%

rem Don't try to create the folder if it already exists.
if not exist "%CONFIGURATION%\%ARCHITECTURE%" (
    mkdir %CONFIGURATION%\%ARCHITECTURE%
)

set "DESTINATION=%CONFIGURATION%\%ARCHITECTURE%"

if not exist "data\shaders\compiled\mainPost_vs.hlsl" (
    goto ERR_SHADER_COMPILATION_FAILED
)

echo.
echo [Copying build products...]
copy /y /b t2-output\%TUNDRATARGET%\aliasIsolation.dll %DESTINATION%\aliasIsolation.asi
copy /y /b t2-output\%TUNDRATARGET%\aliasIsolation.pdb %DESTINATION%
rem copy /y /b t2-output\%TUNDRATARGET%\cinematicTools.dll %DESTINATION%
rem copy /y /b t2-output\%TUNDRATARGET%\cinematicTools.pdb %DESTINATION%

rem Don't try to create the mods folder if it already exists.
if not exist "%DESTINATION%\mods" (
    mkdir %DESTINATION%\mods
)
if not exist "%DESTINATION%\mods\aliasIsolation" (
    mkdir %DESTINATION%\mods\aliasIsolation
)
if not exist "%DESTINATION%\mods\aliasIsolation\data" (
    mkdir %DESTINATION%\mods\aliasIsolation\data
)
if not exist "%DESTINATION%\mods\aliasIsolation\data\textures" (
    mkdir %DESTINATION%\mods\aliasIsolation\data\textures
)
if not exist "%DESTINATION%\mods\aliasIsolation\data\shaders" (
    mkdir %DESTINATION%\mods\aliasIsolation\data\shaders
)

echo.
echo [Copying data files...]
xcopy /Y /S /E data\shaders\compiled %DESTINATION%\mods\aliasIsolation\data\shaders
xcopy /Y /S /E data\textures %DESTINATION%\mods\aliasIsolation\data\textures

echo.
echo [Copying README file...]
copy /a /y README.md %DESTINATION%

goto END

:ERR_INVALID_BUILD_TARGET
rem Alert the user that the configuration they requested doesn't exist.
echo [Build failed] Target configuration "%1" does not exist.
echo Only targets "release" and "debug" are currently supported.
exit 1

:ERR_INVALID_BUILD_ARCHITECTURE
rem Alert the user that the architecture they requested is not supported.
echo [Build failed] Target architecture "%2" is not supported.
echo Only architectures "x86" and "x64" are currently supported.
exit 1

:ERR_SHADER_COMPILATION_FAILED
rem Alert the user that the shaders failed to compile.
echo [Build failed] Shader compilation failed, could not locate compiled shader files.
exit 1

:END
echo.
echo All done.
pause

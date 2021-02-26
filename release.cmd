@echo off
setlocal enabledelayedexpansion

rem If we are provided with a build configuration, then use that.
if "%1" == "release" (
	set "CONFIGURATION=release"
) else if "%1" == "debug" (
	rem Default the build to release if we have no parameter stating otherwise.
	set "CONFIGURATION=debug"
) else if "%1" == "" (
    rem If we are given no configuration at all, assume we wanted release.
    set "CONFIGURATION=release"
) else (
    goto ERR_INVALID_BUILD_TARGET
)

rem Compile the project with the chosen configuration.
call compile.cmd %CONFIGURATION%

rem Don't try to create the folder if it already exists.
if not exist "%CONFIGURATION%" (
    mkdir %CONFIGURATION%
)

echo.
echo [Copying build products...]
xcopy /Y t2-output\win32-msvc-%CONFIGURATION%-default\aliasIsolation.dll %CONFIGURATION%
xcopy /Y t2-output\win32-msvc-%CONFIGURATION%-default\aliasIsolation.pdb %CONFIGURATION%
xcopy /Y t2-output\win32-msvc-%CONFIGURATION%-default\aliasIsolationInjector.exe %CONFIGURATION%
xcopy /Y t2-output\win32-msvc-%CONFIGURATION%-default\aliasIsolationInjector.pdb %CONFIGURATION%
xcopy /Y t2-output\win32-msvc-%CONFIGURATION%-default\aliasIsolationInjectorGui.exe %CONFIGURATION%
xcopy /Y t2-output\win32-msvc-%CONFIGURATION%-default\aliasIsolationInjectorGui.pdb %CONFIGURATION%
xcopy /Y t2-output\win32-msvc-%CONFIGURATION%-default\cinematicTools.dll %CONFIGURATION%
xcopy /Y t2-output\win32-msvc-%CONFIGURATION%-default\cinematicTools.pdb %CONFIGURATION%

rem Don't try to create the data folder if it already exists.
if not exist "%CONFIGURATION%\data" (
    mkdir %CONFIGURATION%\data
)

echo.
echo [Copying data files...]
xcopy /Y /S /E data %CONFIGURATION%\data

echo.
echo [Copying README file...]
xcopy /Y README.txt %CONFIGURATION%

echo.
echo [Creating detachAll.cmd...]
echo aliasIsolationInjector detach > %CONFIGURATION%\detachAll.cmd

goto END

:ERR_INVALID_BUILD_TARGET
rem Alert the user that the configuration they requested doesn't exist.
echo [Build failed] Target configuration "%1" does not exist.
echo Only targets "release" and "debug" are currently supported.

:END
echo.
echo All done.
pause
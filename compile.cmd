@echo off
setlocal enabledelayedexpansion

rem If we are provided with a build configuration, then use that.
if not "%1" == "" (
	set "CONFIGURATION=%1"
) else (
	rem Default the build to release if we have no parameter stating otherwise.
	set "CONFIGURATION=release"
)

rem x64 builds are not yet supported, force the build target architecture to x86.
set "ARCHITECTURE=x86"

rem Check for VSWhere on an x64 system and populate the VSWHEREPATH variable with that value.
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
	set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
) else if exist "%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe" (
	set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
) else (
	goto ERR_NO_VSWHERE
)

rem Get the path to the latest version of Visual Studio's MSBuild on the target system.
for /f "usebackq tokens=*" %%p in (`"%VSWHERE%" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
	set "MSBUILD=%%p"
)

rem Do we have a non-empty MSBuild path?
if defined %MSBUILD% (
	echo --------------[Build Configuration]--------------
	echo Building the project with the following settings:
	echo 	Configuration: %CONFIGURATION%
	echo 	Architecture:  %ARCHITECTURE%
	echo 	VSWhere Path:  %VSWHERE%
	echo 	MSBuild Path:  %MSBUILD%
	echo.
	echo.

	rem Make sure our Boost build script exists before continuing.
	if not exist "src\external\boost\build_boost.bat" (
		goto ERR_NO_BUILD_BOOST_BAT
	)

	echo [Building Boost...]
	rem Enter the Boost directory temporarily.
	pushd src\external\boost
	@echo off
	call .\build_boost.bat %CONFIGURATION%
	popd

	echo.
	echo [Building DirectXTK...]
	rem DirectXTK sets _WIN32_WINNT to 0x0A00 (Windows 10 minimum) in their Win10 solution, which may affect our compatability with Windows 7 users.
	rem We use the Win7 version of the solution so we get a build that still supports Windows 7.
	rem FX11 does not have this issue as they set _WIN32_WINNT to 0x0601 (Windows 7 minimum) like we do.
	"%MSBUILD%" -nologo -m -target:DirectXTK_Desktop_2019 -property:Configuration=%CONFIGURATION%;Platform=%ARCHITECTURE% src/external/DirectXTK/DirectXTK_Desktop_2019_Win7.sln

	echo.
	echo [Building FX11...]
	rem HACK! FX11 uses a different platform name for x86 from DirectXTK.
	rem Force it to Win32 if we are told to use x86, otherwise continue with the passed platform name (as they use x64 across both DirectXTK and FX11's project files).
	if "%ARCHITECTURE%" == "x86" (
		"%MSBUILD%" -nologo -m -target:Effects11 -property:Configuration=%CONFIGURATION%;Platform=Win32 src/external/FX11/Effects11_2019_Win10.sln
	) else (
		"%MSBUILD%" -nologo -m -target:Effects11 -property:Configuration=%CONFIGURATION%;Platform=%ARCHITECTURE% src/external/FX11/Effects11_2019_Win10.sln
	)

	echo.
	echo [Building Alias Isolation...]
	tools\tundra2\bin\tundra2 %CONFIGURATION%

	goto END
) else (
	goto ERR_NO_MSBUILD
)

:ERR_NO_VSWHERE
echo.
echo [Build failed] Failed to find vswhere.exe. Do you have Visual Studio installed?
goto END

:ERR_NO_MSBUILD
echo.
echo [Build failed] Failed to find MSBuild.exe. Do you have Visual Studio installed?
goto END

:ERR_NO_BUILD_BOOST_BAT
echo.
echo [Build failed] Failed to find build_boost.bat in src\external\boost. Are you running this script from the right place?

:END
echo.
echo Finished build.
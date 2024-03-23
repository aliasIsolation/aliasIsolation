@echo off
setlocal enabledelayedexpansion

rem If we are provided with a build configuration, then use that.
if "%1" == "debug" (
	set "CONFIGURATION=debug"
) else (
	rem Default the build to release if we have no parameter stating otherwise.
	set "CONFIGURATION=release"
)

rem If we are provided with a build architecture, then use that.
if "%2" == "x64" (
	set "ARCHITECTURE=x64"
) else (
	rem Default the build to x86 for now if we have no parameter stating otherwise.
	set "ARCHITECTURE=x86"
)

rem Set extra options to be passed to Tundra.
set "TUNDRAEXTRAOPTS=%3"

rem Check for VSWhere on an x64 system and populate the VSWHEREPATH variable with that value.
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
	set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
) else if exist "%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe" (
	set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
) else (
	goto ERR_NO_VSWHERE
)

rem Get the path to the latest version of Visual Studio's MSBuild on the target system.
rem "-products *" instructs vswhere to find all VS products, not just VS IDE installations.
for /f "usebackq tokens=*" %%p in (`call "%VSWHERE%" -products * -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
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
	echo 	Extra Options: %TUNDRAEXTRAOPTS%
	echo.
	echo.

	rem Make sure our Boost build script exists before continuing.
	rem if not exist "src\external\boost\build_boost.bat" (
	rem	goto ERR_NO_BUILD_BOOST_BAT
	rem )

	rem echo [Building Boost...]
	rem Enter the Boost directory temporarily.
	rem pushd src\external\boost
	rem @echo off
	rem call .\build_boost.bat %CONFIGURATION% %ARCHITECTURE%
	rem popd

    rem This is a dirty hack, but I don't want to fork the DirectXTK and FX11 repos just to change
    rem a VS build flag.
    rem if "%CONFIGURATION%" == "debug" (
    rem    set "_CL_=/MTd"
    rem ) else (
    rem    set "_CL_=/MT"
    rem )

	rem echo.
	rem echo [Building DirectXTK...]
	rem DirectXTK sets _WIN32_WINNT to 0x0A00 (Windows 10 minimum) in their Win10 solution, which may affect our compatability with Windows 7 users.
	rem We use the Win7 version of the solution so we get a build that still supports Windows 7.
	rem FX11 does not have this issue as they set _WIN32_WINNT to 0x0601 (Windows 7 minimum) like we do.
	rem "%MSBUILD%" -nologo -m -target:DirectXTK_Desktop_2022 -property:Configuration=%CONFIGURATION%;Platform=%ARCHITECTURE% src/external/DirectXTK/DirectXTK_Desktop_2022_Win7.sln

	rem If we do not have an errorlevel of 0, then something went wrong during the DirectXTK build.
	rem if not %ERRORLEVEL% == 0 (
	rem	   echo [Build failed] DirectXTK failed to build.
	rem	   exit %ERRORLEVEL%
	rem )

	rem echo.
	rem echo [Building FX11...]
	rem HACK! FX11 uses a different platform name for x86 from DirectXTK.
	rem Force it to Win32 if we are told to use x86, otherwise continue with the passed platform name (as they use x64 across both DirectXTK and FX11's project files).
	rem if "%ARCHITECTURE%" == "x86" (
	rem	   "%MSBUILD%" -nologo -m -target:Effects11 -property:Configuration=%CONFIGURATION%;Platform=Win32 src/external/FX11/Effects11_2022_Win10.sln
	rem ) else (
	rem	   "%MSBUILD%" -nologo -m -target:Effects11 -property:Configuration=%CONFIGURATION%;Platform=%ARCHITECTURE% src/external/FX11/Effects11_2022_Win10.sln
	rem )

	rem If we do not have an errorlevel of 0, then something went wrong during the FX11 build.
	rem if not %ERRORLEVEL% == 0 (
	rem	   echo [Build failed] FX11 failed to build.
	rem	   exit %ERRORLEVEL%
	rem )

	echo.
	echo [Building Alias Isolation...]
	if "%ARCHITECTURE%" == "x64" (
		tools\tundra2\bin\tundra2.exe %TUNDRAEXTRAOPTS% win64-msvc-%CONFIGURATION%-default
	) else (
		tools\tundra2\bin\tundra2.exe %TUNDRAEXTRAOPTS% win32-msvc-%CONFIGURATION%-default
	)

    rem If we do not have an errorlevel of 0, then something went wrong during the Tundra build.
	if not %ERRORLEVEL% == 0 (
		echo [Build failed] Alias Isolation failed to build during the Tundra build stage.
		exit %ERRORLEVEL%
	)

    rem This is a dirty, dirty hack. Microsoft's vcvars batch files don't seem to properly set up the Windows Kits paths, so we have to hard code this...
	"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x86\fxc.exe" /nologo /Tps_4_0 /EmainPS /O3 /Ges /Qstrip_reflect /Qstrip_debug /Fo "data/shaders/compiled/sharpen_ps.hlsl" "data/shaders/sharpen_ps.hlsl"
	"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x86\fxc.exe" /nologo /Tps_4_0 /EmainPS /O3 /Ges /Qstrip_reflect /Qstrip_debug /Fo "data/shaders/compiled/shadowLinearize_ps.hlsl" "data/shaders/shadowLinearize_ps.hlsl"
	"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x86\fxc.exe" /nologo /Tps_4_0 /EmainPS /O3 /Ges /Qstrip_reflect /Qstrip_debug /Fo "data/shaders/compiled/chromaticAberration_ps.hlsl" "data/shaders/chromaticAberration_ps.hlsl"
	"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x86\fxc.exe" /nologo /Tps_4_0 /EmainPS /O3 /Ges /Qstrip_reflect /Qstrip_debug /Fo "data/shaders/compiled/bloomMerge_ps.hlsl" "data/shaders/bloomMerge_ps.hlsl"
	"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x86\fxc.exe" /nologo /Tps_4_0 /EmainPS /O3 /Ges /Qstrip_reflect /Qstrip_debug /Fo "data/shaders/compiled/shadowDownsample_ps.hlsl" "data/shaders/shadowDownsample_ps.hlsl"
	"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x86\fxc.exe" /nologo /Tvs_4_0 /EmainVS /O3 /Ges /Qstrip_reflect /Qstrip_debug /Fo "data/shaders/compiled/mainPost_vs.hlsl" "data/shaders/mainPost_vs.hlsl"

	rem If we do not have an errorlevel of 0, then something went wrong during the shader compilation.
	if not %ERRORLEVEL% == 0 (
		echo [Build failed] Failed to compile the shaders for Alias Isolation.
		exit %ERRORLEVEL%
	)

	goto END
) else (
	goto ERR_NO_MSBUILD
)

:ERR_NO_VSWHERE
echo.
echo [Build failed] Failed to find vswhere.exe. Do you have Visual Studio installed?
exit 2

:ERR_NO_MSBUILD
echo.
echo [Build failed] Failed to find MSBuild.exe. Do you have Visual Studio installed?
exit 2

rem :ERR_NO_BUILD_BOOST_BAT
rem echo.
rem echo [Build failed] Failed to find build_boost.bat in src\external\boost. Are you running this script from the right place?
rem exit 2

:END
echo.
echo [Build success] Build complete.

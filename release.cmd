@echo off
call compile.cmd
mkdir release
xcopy /Y t2-output\win32-msvc-release-default\aliasIsolation.dll release
xcopy /Y t2-output\win32-msvc-release-default\aliasIsolationInjector.exe release
mkdir release\data
xcopy /Y /S /E data release\data
xcopy /Y README.md release
echo aliasIsolationInjector detach > release\detachAll.cmd
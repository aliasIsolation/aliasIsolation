@echo off
call compile_debug.cmd
mkdir debug
xcopy /Y t2-output\win32-msvc-debug-default\aliasIsolation.dll debug
xcopy /Y t2-output\win32-msvc-debug-default\aliasIsolation.pdb debug
xcopy /Y t2-output\win32-msvc-debug-default\aliasIsolationInjector.exe debug
xcopy /Y t2-output\win32-msvc-debug-default\aliasIsolationInjector.pdb debug
xcopy /Y t2-output\win32-msvc-debug-default\aliasIsolationInjectorGui.exe debug
xcopy /Y t2-output\win32-msvc-debug-default\aliasIsolationInjectorGui.pdb debug
xcopy /Y t2-output\win32-msvc-debug-default\cinematicTools.dll debug
xcopy /Y t2-output\win32-msvc-debug-default\cinematicTools.pdb debug
mkdir debug\data
xcopy /Y /S /E data debug\data
xcopy /Y README.txt debug
echo aliasIsolationInjector detach > debug\detachAll.cmd
pause

#pragma once

#include <windows.h>
#include <string>

void injectDll(HANDLE hTarget, const char* const modulePath);
bool isAlreadyInjected(const HANDLE proc, const std::string& dllPath);

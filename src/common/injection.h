#pragma once

#include <windows.h>

void injectDll(HANDLE hTarget, const char* const modulePath);

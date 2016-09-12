#pragma once

void* hookMethod(void** vtbl, int fnIndex, void* newFn, void** oldFn);
void enableMethodHook(void* hookHandle);
void disableMethodHook(void* hookHandle);

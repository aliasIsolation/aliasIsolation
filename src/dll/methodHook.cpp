#include "methodHook.h"

#include "common.h"
#include "MinHook.h"

#include <array>

struct HookData {
	void** vtblEntryPtr;
	void* newFn;
	void* oldFn;
};

enum { MaxHooks = 32 };
std::array<HookData, MaxHooks>	g_methodHooks;
int								g_methodHookCount = 0;

void* hookMethod(void** vtbl, int fnIndex, void* newFn, void** oldFn)
{
	if (g_methodHookCount >= MaxHooks) DebugBreak();
	HookData& hd = g_methodHooks[g_methodHookCount++];
	hd.vtblEntryPtr = vtbl + fnIndex;
	hd.newFn = newFn;
	MH_CHECK(MH_CreateHook(*hd.vtblEntryPtr, newFn, &hd.oldFn));
	*oldFn = hd.oldFn;
	return &hd;
}

void enableMethodHook(void* hookHandle)
{
	if (!hookHandle) return;

	const HookData& hd = *(const HookData*)hookHandle;
	MH_EnableHook(*hd.vtblEntryPtr);
}

void disableMethodHook(void* hookHandle)
{
	if (!hookHandle) return;

	const HookData& hd = *(const HookData*)hookHandle;
	MH_DisableHook(*hd.vtblEntryPtr);
}

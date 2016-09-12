#include "methodHook.h"
#include "common.h"
#include "MinHook.h"
#include <array>


struct HookData {
	bool intrusive;
	void** vtblEntryPtr;
	void* newFn;
	void* oldFn;
};

enum { MaxHooks = 32 };
std::array<HookData, MaxHooks>	g_methodHooks;
int								g_methodHookCount = 0;

void* intrusiveHookMethod(void** vtbl, int fnIndex, void* newFn, void** oldFn)
{
	if (g_methodHookCount >= MaxHooks) DebugBreak();
	HookData& hd = g_methodHooks[g_methodHookCount++];
	hd.vtblEntryPtr = vtbl + fnIndex;
	hd.newFn = newFn;
	hd.oldFn = *oldFn = *hd.vtblEntryPtr;
	hd.intrusive = true;
	return &hd;
}

void* hookMethod(void** vtbl, int fnIndex, void* newFn, void** oldFn)
{
#if 0
	return intrusiveHookMethod(vtbl, fnIndex, newFn, oldFn);
#else
	if (g_methodHookCount >= MaxHooks) DebugBreak();
	HookData& hd = g_methodHooks[g_methodHookCount++];
	hd.vtblEntryPtr = vtbl + fnIndex;
	hd.newFn = newFn;
	MH_CHECK(MH_CreateHook(*hd.vtblEntryPtr, newFn, &hd.oldFn));
	*oldFn = hd.oldFn;
	hd.intrusive = false;
	return &hd;
#endif
}

void enableMethodHook(void* hookHandle)
{
	const HookData& hd = *(const HookData*)hookHandle;
	if (hd.intrusive)
	{
		DWORD origPageProtect;
		VirtualProtect(hd.vtblEntryPtr, 4, PAGE_EXECUTE_READWRITE, &origPageProtect);
		*hd.vtblEntryPtr = hd.newFn;
		VirtualProtect(hd.vtblEntryPtr, 4, origPageProtect, &origPageProtect);
	}
	else
	{
		MH_EnableHook(*hd.vtblEntryPtr);
	}
}

void disableMethodHook(void* hookHandle)
{
	const HookData& hd = *(const HookData*)hookHandle;
	if (hd.intrusive)
	{
		DWORD origPageProtect;
		VirtualProtect(hd.vtblEntryPtr, 4, PAGE_EXECUTE_READWRITE, &origPageProtect);
		*hd.vtblEntryPtr = hd.oldFn;
		VirtualProtect(hd.vtblEntryPtr, 4, origPageProtect, &origPageProtect);
	}
	else
	{
		MH_DisableHook(*hd.vtblEntryPtr);
	}
}

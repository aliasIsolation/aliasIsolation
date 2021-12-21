#pragma once

// Include STDIO for sprintf.
#include <stdio.h>

typedef unsigned int uint;

#define MH_CHECK(EXPR) { int ret = EXPR; if (ret != MH_OK) { char buf[256]; sprintf(buf, #EXPR " failed: %d.", ret); MessageBoxA(NULL, buf, NULL, NULL); DebugBreak(); } }
#define DX_CHECK(EXPR) { HRESULT ret = EXPR; if (ret != S_OK) { char buf[256]; sprintf(buf, #EXPR " failed: 0x%x.", ret); MessageBoxA(NULL, buf, NULL, NULL); DebugBreak(); } }
#ifdef _DEBUG
	#define LOG_MSG(EXPR, FMTEXPR) { printf_s(EXPR, FMTEXPR); }
#else
	// Strip out the debug messages if we are not targeting debug.
	#define LOG_MSG(EXPR, FMTEXPR) {  }
#endif

struct ShaderHandle {
	size_t	id;

	bool operator==(const ShaderHandle& rhs) const {
		return id == rhs.id;
	}

	bool isValid() const { return id != ~size_t(0); }

	ShaderHandle() : id(~size_t(0)) {}
	explicit ShaderHandle(size_t id_) : id(id_) {}
};

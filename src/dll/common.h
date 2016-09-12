#pragma once

typedef unsigned int uint;

#define MH_CHECK(EXPR) { int ret = EXPR; if (ret != MH_OK) { char buf[256]; sprintf(buf, #EXPR " failed: %d.", ret); MessageBoxA(NULL, buf, NULL, NULL); DebugBreak(); } }
#define DX_CHECK(EXPR) { HRESULT ret = EXPR; if (ret != S_OK) { MessageBoxA(NULL, #EXPR " failed.", NULL, NULL); DebugBreak(); } }


struct ShaderHandle {
	size_t	id;

	bool operator==(const ShaderHandle& rhs) const {
		return id == rhs.id;
	}

	bool isValid() const { return id != ~size_t(0); }

	ShaderHandle() : id(~size_t(0)) {}
	explicit ShaderHandle(size_t id_) : id(id_) {}
};

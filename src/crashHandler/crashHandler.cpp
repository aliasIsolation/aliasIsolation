// http://stackoverflow.com/questions/15372962/in-the-handler-of-setunhandledexceptionfilter-how-to-print-a-stacktrace

#include <stdexcept>
#include <iterator>

#include <windows.h>
#include <psapi.h>

// 'typedef ': ignored on left of '' when no variable is declared
#pragma warning(disable : 4091)

// Some versions of imagehlp.dll lack the proper packing directives themselves
// so we need to do it.
#pragma pack( push, before_imagehlp, 8 )
#include <imagehlp.h>
#pragma pack( pop, before_imagehlp )

#include <vector>
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <sstream>

struct module_data {
	std::string image_name;
	std::string module_name;
	void *base_address;
	DWORD load_size;
};
typedef std::vector<module_data> ModuleList;

HANDLE thread_ready;

LONG WINAPI Filter( EXCEPTION_POINTERS *ep );

void show_stack(std::ostream &, HANDLE hThread, CONTEXT& c);
void *load_modules_symbols( HANDLE hProcess, DWORD pid );

void installCrashHandler()
{
	SetUnhandledExceptionFilter(&Filter);
}

void uninstallCrashHandler()
{
	SetUnhandledExceptionFilter(nullptr);
}

bool WriteFullDump(HANDLE hProc, EXCEPTION_POINTERS *ep, const char* filePath)
{
	HANDLE hFile = CreateFile(filePath, GENERIC_ALL, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (!hFile)
	{
		return false;
	}
	else
	{
		MINIDUMP_EXCEPTION_INFORMATION mdei; 

		mdei.ThreadId           = GetCurrentThreadId(); 
		mdei.ExceptionPointers  = ep; 
		mdei.ClientPointers     = FALSE;

		BOOL Result = MiniDumpWriteDump( hProc,
			GetProcessId(hProc),
			hFile,
			(MINIDUMP_TYPE)(MiniDumpNormal),
			&mdei,
			nullptr,
			nullptr );

		CloseHandle(hFile);

		if (!Result)
		{
			return false;
		}
	}

	return true;
}

// if you use C++ exception handling: install a translator function
// with set_se_translator(). In the context of that function (but *not*
// afterwards), you can either do your stack dump, or save the CONTEXT
// record as a local copy. Note that you must do the stack dump at the
// earliest opportunity, to avoid the interesting stack-frames being gone
// by the time you do the dump.
LONG WINAPI Filter(EXCEPTION_POINTERS *ep) {
	HANDLE thread;

	std::string dumpFilePath;
	{
		char tempPath[MAX_PATH];
		GetTempPathA(MAX_PATH, tempPath);
		dumpFilePath = std::string(tempPath) + "aliasIsolationCrashDump.dmp";
	}

	const bool dumpWritten = WriteFullDump(GetCurrentProcess(), ep, dumpFilePath.c_str());

	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(),
		GetCurrentProcess(), &thread, 0, false, DUPLICATE_SAME_ACCESS);

	std::stringstream stackTrace;

	stackTrace << "A fatal error was caught by Alias Isolation, and the current process will terminate.\n";
	if (dumpWritten) {
		stackTrace << "A crash dump has been written to " << dumpFilePath << ".\n";
	}
	stackTrace << "Press Ctrl+C to copy the contents of this message to the clipboard.\n\n";

	show_stack(stackTrace, thread, *(ep->ContextRecord));
	CloseHandle(thread);

	MessageBoxA(NULL, stackTrace.str().c_str(), "Fatal error", MB_ICONEXCLAMATION);

	return EXCEPTION_EXECUTE_HANDLER;
}

class SymHandler { 
	HANDLE p;
public:
	SymHandler(HANDLE process, char const *path=NULL, bool intrude = false) : p(process) { 
		SymInitialize(p, path, intrude);
	}
	~SymHandler() { SymCleanup(p); }
};

#ifdef _M_X64
STACKFRAME64 init_stack_frame(CONTEXT c) {
	STACKFRAME64 s;
	s.AddrPC.Offset = c.Rip;
	s.AddrPC.Mode = AddrModeFlat;
	s.AddrStack.Offset = c.Rsp;
	s.AddrStack.Mode = AddrModeFlat;    
	s.AddrFrame.Offset = c.Rbp;
	s.AddrFrame.Mode = AddrModeFlat;
	return s;
}
#else
STACKFRAME64 init_stack_frame(CONTEXT c) {
	STACKFRAME64 s;
	s.AddrPC.Offset = c.Eip;
	s.AddrPC.Mode = AddrModeFlat;
	s.AddrStack.Offset = c.Esp;
	s.AddrStack.Mode = AddrModeFlat;    
	s.AddrFrame.Offset = c.Ebp;
	s.AddrFrame.Mode = AddrModeFlat;
	return s;
}
#endif

void sym_options(DWORD add, DWORD remove=0) {
	DWORD symOptions = SymGetOptions();
	symOptions |= add;
	symOptions &= ~remove;
	SymSetOptions(symOptions);
}

class symbol { 
	typedef IMAGEHLP_SYMBOL64 sym_type;
	sym_type *sym;
	static const int max_name_len = 1024;
public:
	symbol(HANDLE process, DWORD64 address) : sym((sym_type *)::operator new(sizeof(*sym) + max_name_len)) {
		memset(sym, '\0', sizeof(*sym) + max_name_len);
		sym->SizeOfStruct = sizeof(*sym);
		sym->MaxNameLength = max_name_len;
		DWORD64 displacement;

		if (!SymGetSymFromAddr64(process, address, &displacement, sym))
		{
			delete sym;
		}
	}

	std::string name() { return sym ? std::string(sym->Name) : "???"; }
	std::string undecorated_name() { 
		if (!sym) return "???";
		std::vector<char> und_name(max_name_len);
		UnDecorateSymbolName(sym->Name, &und_name[0], max_name_len, UNDNAME_COMPLETE);
		return std::string(&und_name[0], strlen(&und_name[0]));
	}
};

void show_stack(std::ostream &os, HANDLE hThread, CONTEXT& c) {
	HANDLE process = GetCurrentProcess();
	
	DWORD offset_from_symbol=0;
	IMAGEHLP_LINE64 line = {0};

	//SymHandler handler(process, g_symSearchPath.c_str(), true);
	SymHandler handler(process);

	sym_options(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);

	void *base = load_modules_symbols(process, GetCurrentProcessId());

	STACKFRAME64 s = init_stack_frame(c);

	line.SizeOfStruct = sizeof line;

	IMAGE_NT_HEADERS *h = ImageNtHeader(base);
	DWORD image_type = h->FileHeader.Machine;

	for (int frame_number = 0;; ++frame_number) {
		if (frame_number > 0)
		{
			if (!StackWalk64(image_type, process, hThread, &s, &c, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
			{
				return;
			}
		}

		if (s.AddrPC.Offset != 0) {
			os << symbol(process, s.AddrPC.Offset).undecorated_name();

			if (SymGetLineFromAddr64( process, s.AddrPC.Offset, &offset_from_symbol, &line ) ) 
				os << " " << line.FileName << " (" << line.LineNumber << ")";

			os << std::endl;
		}
	}
}

class get_mod_info {
	HANDLE process;
	static const int buffer_length = 4096;
public:
	get_mod_info(HANDLE h) : process(h) {}

	module_data operator()(HMODULE module) { 
		module_data ret;
		char temp[buffer_length];
		MODULEINFO mi;

		GetModuleInformation(process, module, &mi, sizeof(mi));
		ret.base_address = mi.lpBaseOfDll;
		ret.load_size = mi.SizeOfImage;

		GetModuleFileNameEx(process, module, temp, sizeof(temp));
		ret.image_name = temp;
		GetModuleBaseName(process, module, temp, sizeof(temp));
		ret.module_name = temp;
		std::vector<char> img(ret.image_name.begin(), ret.image_name.end());
		std::vector<char> mod(ret.module_name.begin(), ret.module_name.end());
		SymLoadModule64(process, 0, &img[0], &mod[0], (DWORD64)ret.base_address, ret.load_size);
		return ret;
	}
};

void *load_modules_symbols(HANDLE process, DWORD pid) {
	ModuleList modules;

	DWORD cbNeeded;
	std::vector<HMODULE> module_handles(1);

	EnumProcessModules(process, &module_handles[0], module_handles.size() * sizeof(HMODULE), &cbNeeded);
	module_handles.resize(cbNeeded/sizeof(HMODULE));
	EnumProcessModules(process, &module_handles[0], module_handles.size() * sizeof(HMODULE), &cbNeeded);

	std::transform(module_handles.begin(), module_handles.end(), std::back_inserter(modules), get_mod_info(process));
	return modules[0].base_address;
}
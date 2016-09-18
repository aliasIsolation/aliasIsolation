#pragma once
#include <string>
#include <fstream>
#include <windows.h>
#include <sstream>

using namespace std;

class Log
{
public:
	enum Type{ NORMAL = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY, ERR = FOREGROUND_RED | FOREGROUND_INTENSITY, WARNING = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, OK = FOREGROUND_GREEN | FOREGROUND_INTENSITY};
	enum DebugMode{ NONE, CONSOLE, CONSOLE_AND_FILE };
	static void Init(DebugMode mode);
	static void DeInit();
	static void Write(std::string);
	static void WriteWarning(std::string);
	static void WriteError(std::string);
	static void WriteMaGiC();
	static void WriteOK(std::string);
	static void WriteInput(std::string);

private:
	static std::fstream stream;
	static DebugMode mode;

public:
	template <typename T>
	static std::string convertToHexa(T val)
	{
		INT p = (INT)val;
		std::stringstream str;
		str << uppercase << hex << p;
		return str.str();
	}
};
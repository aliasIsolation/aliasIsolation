#include "Log.h"
#include "boost\date_time\gregorian\gregorian.hpp"
#include "boost\date_time\posix_time\posix_time.hpp"

using namespace std;
using namespace boost::posix_time;
using namespace boost::gregorian;

fstream Log::stream;
HANDLE hstdin;
HANDLE hstdout;
Log::DebugMode Log::mode;

time_facet* facet;
ostringstream is;

void Log::Init(Log::DebugMode Mode)
{
	mode = Mode;
	if(mode >= Log::DebugMode::CONSOLE)
	{
		facet = new time_facet("%d-%m-%y %T");
		is.imbue(std::locale(is.getloc(), facet));
		ptime now = second_clock::local_time();
		if(mode == Log::DebugMode::CONSOLE_AND_FILE)
		{
			string filename = "AI Log " + to_simple_string(now.date()) + ".txt";
			stream.open(filename, std::fstream::out);
		}
		hstdin  = GetStdHandle( STD_INPUT_HANDLE  );
		hstdout = GetStdHandle( STD_OUTPUT_HANDLE );
	}
}

void Log::DeInit()
{
	stream.close();
}

void Log::Write(string msg)
{
	if(mode >= Log::DebugMode::CONSOLE)
	{
		ptime now = second_clock::local_time();
		is << now;
		string timeStamp = is.str();
		is.str("");
		is.clear();
		string message = "  " + msg + "\n";
		SetConsoleTextAttribute(hstdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
		printf(timeStamp.c_str());
		SetConsoleTextAttribute( hstdout, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY );
		printf(message.c_str());
		if(mode == Log::DebugMode::CONSOLE_AND_FILE)
		{
			stream.write(timeStamp.c_str(), timeStamp.size());
			stream.write(message.c_str(), message.size());
		}
	}
}

void Log::WriteWarning(string msg)
{
	if(mode >= Log::DebugMode::CONSOLE)
	{
		ptime now = second_clock::local_time();
		is << now;
		string timeStamp = is.str();
		is.str("");
		is.clear();
		string message = "  " + msg + "\n";
		SetConsoleTextAttribute(hstdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
		printf(timeStamp.c_str());
		SetConsoleTextAttribute( hstdout, Log::Type::WARNING );
		printf(message.c_str());
		if(mode == Log::DebugMode::CONSOLE_AND_FILE)
		{
			stream.write(timeStamp.c_str(), timeStamp.size());
			stream.write(message.c_str(), message.size());
		}
	}
}

void Log::WriteError(string msg)
{
	if(mode >= Log::DebugMode::CONSOLE)
	{
		ptime now = second_clock::local_time();
		is << now;
		string timeStamp = is.str();
		is.str("");
		is.clear();
		string message = "  " + msg + "\n";
		SetConsoleTextAttribute(hstdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
		printf(timeStamp.c_str());
		SetConsoleTextAttribute( hstdout, Log::Type::ERR );
		printf(message.c_str());
		if(mode == Log::DebugMode::CONSOLE_AND_FILE)
		{
			stream.write(timeStamp.c_str(), timeStamp.size());
			stream.write(message.c_str(), message.size());
		}
	}
}

void Log::WriteOK(string msg)
{
	if(mode >= Log::DebugMode::CONSOLE)
	{
		ptime now = second_clock::local_time();
		is << now;
		string timeStamp = is.str();
		is.str("");
		is.clear();
		string message = "  " + msg + "\n";
		SetConsoleTextAttribute(hstdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
		printf(timeStamp.c_str());
		SetConsoleTextAttribute( hstdout, Log::Type::OK );
		printf(message.c_str());
		if(mode == Log::DebugMode::CONSOLE_AND_FILE)
		{
			stream.write(timeStamp.c_str(), timeStamp.size());
			stream.write(message.c_str(), message.size());
		}
	}
}

void Log::WriteInput(string msg)
{
	if (mode >= Log::DebugMode::CONSOLE)
	{
		ptime now = second_clock::local_time();
		is << now;
		string timeStamp = is.str();
		is.str("");
		is.clear();
		string message = "  " + msg;
		SetConsoleTextAttribute(hstdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
		printf(timeStamp.c_str());
		SetConsoleTextAttribute(hstdout, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
		printf(message.c_str());
		if (mode == Log::DebugMode::CONSOLE_AND_FILE)
		{
			stream.write(timeStamp.c_str(), timeStamp.size());
			stream.write(message.c_str(), message.size());
		}
	}
}

void Log::WriteMaGiC()
{
	if(mode >= Log::DebugMode::CONSOLE)
	{
		ptime now = second_clock::local_time();
		string message = "MAGIC\n";
		string timeStamp = to_simple_string(now);

		SetConsoleTextAttribute( hstdout, 10 ); printf("\n\t\tM");
		SetConsoleTextAttribute( hstdout, 11 ); printf("A");
		SetConsoleTextAttribute( hstdout, 12 ); printf("G");
		SetConsoleTextAttribute( hstdout, 13 ); printf("I");
		SetConsoleTextAttribute( hstdout, 14 ); printf("C");
		SetConsoleTextAttribute( hstdout, 10 ); printf("%c", 175);
		SetConsoleTextAttribute( hstdout, 11 ); printf("M");
		SetConsoleTextAttribute( hstdout, 12 ); printf("A");
		SetConsoleTextAttribute( hstdout, 13 ); printf("G");
		SetConsoleTextAttribute( hstdout, 14 );
		printf("I");
		SetConsoleTextAttribute( hstdout, 10 );
		printf("C");
		SetConsoleTextAttribute( hstdout, 11 );
		printf("%c", 175);
		SetConsoleTextAttribute( hstdout, 12 );
		printf("M");
		SetConsoleTextAttribute( hstdout, 13 );
		printf("A");
		SetConsoleTextAttribute( hstdout, 14 );
		printf("G");
		SetConsoleTextAttribute( hstdout, 10 );
		printf("I");
		SetConsoleTextAttribute( hstdout, 11 );
		printf("C");
		SetConsoleTextAttribute( hstdout, 10 );
		printf("%c", 175);
		SetConsoleTextAttribute( hstdout, 11 );
		printf("M");
		SetConsoleTextAttribute( hstdout, 12 );
		printf("A");
		SetConsoleTextAttribute( hstdout, 13 );
		printf("G");
		SetConsoleTextAttribute( hstdout, 14 );
		printf("I");
		SetConsoleTextAttribute( hstdout, 10 );
		printf("C");
		SetConsoleTextAttribute( hstdout, 11 );
		printf("%c", 175);
		SetConsoleTextAttribute( hstdout, 12 );
		printf("M");
		SetConsoleTextAttribute( hstdout, 13 );
		printf("A");
		SetConsoleTextAttribute( hstdout, 14 );
		printf("G");
		SetConsoleTextAttribute( hstdout, 10 );
		printf("I");
		SetConsoleTextAttribute( hstdout, 11 );
		printf("C");
		SetConsoleTextAttribute( hstdout, 12 );
		printf("%c", 175);
		SetConsoleTextAttribute( hstdout, 13 );
		printf("M");
		SetConsoleTextAttribute( hstdout, 14 );
		printf("A");
		SetConsoleTextAttribute( hstdout, 10 );
		printf("G");
		SetConsoleTextAttribute( hstdout, 11 );
		printf("I");
		SetConsoleTextAttribute( hstdout, 12 );
		printf("C");
		SetConsoleTextAttribute( hstdout, 13 );
		printf("%c", 175);
		SetConsoleTextAttribute( hstdout, 14 );
		printf("M");
		SetConsoleTextAttribute( hstdout, 10 );
		printf("A");
		SetConsoleTextAttribute( hstdout, 11 );
		printf("G");
		SetConsoleTextAttribute( hstdout, 12 );
		printf("I");
		SetConsoleTextAttribute( hstdout, 13 );
		printf("C");
		SetConsoleTextAttribute( hstdout, 14 );
		printf("%c", 175);
		SetConsoleTextAttribute( hstdout, 10 );
		printf("M");
		SetConsoleTextAttribute( hstdout, 11 );
		printf("A");
		SetConsoleTextAttribute( hstdout, 12 );
		printf("G");
		SetConsoleTextAttribute( hstdout, 13 );
		printf("I");
		SetConsoleTextAttribute( hstdout, 14 );
		printf("C\n");

		SetConsoleTextAttribute( hstdout, 0 ); printf("\t\t");
		SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY );
		printf("             "); SetConsoleTextAttribute( hstdout, BACKGROUND_BLUE);
		printf("        "); SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY );
		printf("                          \n");
		SetConsoleTextAttribute( hstdout, 0 ); printf("\t\t");SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
		printf("             "); SetConsoleTextAttribute( hstdout, BACKGROUND_BLUE);
		printf("        "); SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY );
		printf("                          \n");
		SetConsoleTextAttribute( hstdout, 0 ); printf("\t\t");SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
		printf("             "); SetConsoleTextAttribute( hstdout, BACKGROUND_BLUE);
		printf("        "); SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY );
		printf("                          \n");
		SetConsoleTextAttribute( hstdout, 0 ); printf("\t\t");SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
		printf("             "); SetConsoleTextAttribute( hstdout, BACKGROUND_BLUE);
		printf("        "); SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY );
		printf("                          \n");
		SetConsoleTextAttribute( hstdout, 0 ); printf("\t\t");SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
		printf("             "); SetConsoleTextAttribute( hstdout, BACKGROUND_BLUE);
		printf("        "); SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY );
		printf("                          \n");
		SetConsoleTextAttribute( hstdout, 0 ); printf("\t\t");SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
		printf("             "); SetConsoleTextAttribute( hstdout, BACKGROUND_BLUE);
		printf("        "); SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY );
		printf("                          \n");
		SetConsoleTextAttribute( hstdout, 0 ); printf("\t\t");SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
		printf("             "); SetConsoleTextAttribute( hstdout, BACKGROUND_BLUE);
		printf("        "); SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY );
		printf("                          \n");
		SetConsoleTextAttribute( hstdout, 0 ); printf("\t\t");
		SetConsoleTextAttribute( hstdout, BACKGROUND_BLUE);
		printf("                                               \n");
		SetConsoleTextAttribute( hstdout, 0 ); printf("\t\t");SetConsoleTextAttribute( hstdout, BACKGROUND_BLUE);
		printf("                                               \n");
		SetConsoleTextAttribute( hstdout, 0 ); printf("\t\t");SetConsoleTextAttribute( hstdout, BACKGROUND_BLUE);
		printf("                                               \n");
		SetConsoleTextAttribute( hstdout, 0 ); printf("\t\t");SetConsoleTextAttribute( hstdout, BACKGROUND_BLUE);
		printf("                                               \n");
		SetConsoleTextAttribute( hstdout, 0 ); printf("\t\t");SetConsoleTextAttribute( hstdout, BACKGROUND_BLUE);
		printf("                                               \n");
		SetConsoleTextAttribute( hstdout, 0 ); printf("\t\t");
		SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
		printf("             "); SetConsoleTextAttribute( hstdout, BACKGROUND_BLUE);
		printf("        "); SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY );
		printf("                          \n");
		SetConsoleTextAttribute( hstdout, 0 ); printf("\t\t");SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
		printf("             "); SetConsoleTextAttribute( hstdout, BACKGROUND_BLUE);
		printf("        "); SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY );
		printf("                          \n");
		SetConsoleTextAttribute( hstdout, 0 ); printf("\t\t");SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
		printf("             "); SetConsoleTextAttribute( hstdout, BACKGROUND_BLUE);
		printf("        "); SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY );
		printf("                          \n");
		SetConsoleTextAttribute( hstdout, 0 ); printf("\t\t");SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
		printf("             "); SetConsoleTextAttribute( hstdout, BACKGROUND_BLUE);
		printf("        "); SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY );
		printf("                          \n");
		SetConsoleTextAttribute( hstdout, 0 ); printf("\t\t");SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
		printf("             "); SetConsoleTextAttribute( hstdout, BACKGROUND_BLUE);
		printf("        "); SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY );
		printf("                          \n");
		SetConsoleTextAttribute( hstdout, 0 ); printf("\t\t");SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
		printf("             "); SetConsoleTextAttribute( hstdout, BACKGROUND_BLUE);
		printf("        "); SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY );
		printf("                          \n");
		SetConsoleTextAttribute( hstdout, 0 ); printf("\t\t");SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
		printf("             "); SetConsoleTextAttribute( hstdout, BACKGROUND_BLUE);
		printf("        "); SetConsoleTextAttribute( hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY );
		printf("                          \n");
		SetConsoleTextAttribute( hstdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY ); printf("\n\t\t           Thank you for using ECT!\n\t\t         -Matti \"Hattiwatti\" Hietanen\n\n");

		if(mode == Log::DebugMode::CONSOLE_AND_FILE)
		{
			stream.write(timeStamp.c_str(), timeStamp.size());
			stream.write(message.c_str(), message.size());
		}
	}
}

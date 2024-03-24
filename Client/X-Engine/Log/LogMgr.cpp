#include "EnginePch.h"
#include "LogMgr.h"

#include <ctime>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <iostream>

LogMgr::LogMgr()
{
	AllocConsole();

	freopen_s(&mConsoleStream, "CONOUT$", "w", stdout); // 표준 출력을 콘솔로 리디렉션
	freopen_s(&mConsoleStream, "CONIN$", "r", stdin);	// 표준 입력을 콘솔로 리디렉션
}

LogMgr::~LogMgr()
{
	FreeConsole();
	if (mLogWriteFile.is_open())
		mLogWriteFile.close();
}

std::string LogMgr::LogLevelString(LogLevel level)
{
	switch (level)
	{
	case LogLevel::Trace:
		return "TRACE";
		break;
	case LogLevel::Debug:
		return "DEBUG";
		break;
	case LogLevel::Info:
		return "INFO";
		break;
	case LogLevel::Warning:
		return "WARNING";
		break;
	case LogLevel::Error:
		return "ERROR";
		break;
	case LogLevel::Fatal:
		return "FATAL";
		break;
	default:
		return "";
		break;
	}
	return "";
}

std::string LogMgr::GetCurFileName(std::string& src_File)
{
	if (src_File.size() == 0) return "";

	std::string name{};
	for (int i = src_File.size() - 1; src_File[i] != '\\'; --i)
	{
		name += src_File[i];
	}
	std::reverse(name.begin(), name.end());
	return name;
}

void LogMgr::Sample()
{
	LOG_CONSOLE(LogLevel::Info, "Sample() 함수에서 로그 사용 예시코드를 작성했습니다.\n\n");
		
	LOG_CONSOLE(LogLevel::Basic, "콘솔창에만 로그를 찍습니다.\n");
	
	COUT("로그 레벨 Basic 이며 콘솔창에만 로그 찍습니다.\n");
	
	LOG_CONSOLE(LogLevel::Debug,	"Magenta 로그\n");
	LOG_CONSOLE(LogLevel::Error,	"Red 로그\n");
	LOG_CONSOLE(LogLevel::Fatal,	"Green 로그\n");
	LOG_CONSOLE(LogLevel::Info,		"White 로그\n");
	LOG_CONSOLE(LogLevel::Trace,	"Cyan 로그\n");
	LOG_CONSOLE(LogLevel::Warning,	"Yellow 로그\n");

	LOG_FILE(LogLevel::Debug, "파일에만 로그 작성\n");
	LOG_CONSOLE_FILE(LogLevel::Debug, "콘솔창 과 파일 둘다 로그 작성\n");

}

void LogMgr::LogConsole(LogLevel level, const char* file, const char* func, const int& line, std::string logstr)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	std::string result{};
	std::string logLv = "[ " + LogLevelString(level) + " ]\t";
	std::string filepath{ file };
	std::string filename = LOG_MGR->GetCurFileName(filepath);

	// [ 로그타입 ] 파일::함수명(라인) : 내용
	result = logLv + filename + "::" + func + "(" + std::to_string(line) + ") : " + logstr;

	switch (level)
	{
	case LogLevel::Basic:
	{
		std::cout << logstr;
	}
	break;
	case LogLevel::Trace:
	{
		SetConsoleTextAttribute(hConsole, (DWORD)ConsoleColor::Cyan);
		std::cout << result;
	}
	break;
	case LogLevel::Debug:
	{
		SetConsoleTextAttribute(hConsole, (DWORD)ConsoleColor::Magenta);
		std::cout << result;
	}
	break;
	case LogLevel::Info:
	{
		SetConsoleTextAttribute(hConsole, (DWORD)ConsoleColor::White);
		std::cout << result;
	}
	break;
	case LogLevel::Warning:
	{
		SetConsoleTextAttribute(hConsole, (DWORD)ConsoleColor::Yellow);
		std::cout << result;
	}
	break;
	case LogLevel::Error:
	{
		SetConsoleTextAttribute(hConsole, (DWORD)ConsoleColor::Red);
		std::cout << result;
	}
	break;
	case LogLevel::Fatal:
	{
		SetConsoleTextAttribute(hConsole, (DWORD)ConsoleColor::Green);
		std::cout << result;
	}
	break;
	}

	SetConsoleTextAttribute(hConsole, (DWORD)ConsoleColor::White); // 기본 색상으로 복원
}

void LogMgr::LogFile(LogLevel level, const char* file, const char* func, const int& line, std::string logstr)
{
	if (mLogWriteFile)
	{
		std::string result{};
		std::string logLv = "[ " + LogLevelString(level) + " ]\t";
		std::string filepath{ file };
		std::string filename = LOG_MGR->GetCurFileName(filepath);


		// [ 로그타입 ] 파일::함수명(라인) : 내용
		result = logLv + filename + "::" + func + "(" + std::to_string(line) + ") : " + logstr;
		//mLogWriteFile << result.c_str();
		char c1 = result[result.size() - 1];
		if (c1 == '\n')
		{
			result[result.size() - 1] = NULL;
			mLogWriteFile << result.c_str() << std::endl;
		}
		else
			mLogWriteFile << result.c_str();

	}
}
std::string LogMgr::tmToString(const std::tm& timeStruct, const std::string& format)
{
	std::stringstream ss;
	ss << std::put_time(&timeStruct, format.c_str());
	return ss.str();
}

bool LogMgr::Init(std::string name)
{
	mIsLogWrite = true;

	std::time_t currentTime = std::time(nullptr);
	std::tm		localTime{};	  
	localtime_s(&localTime, &currentTime);

	std::string FileName   = name + " " + tmToString(localTime, "%Y_%m%d_%Hh%Mm%Ss.log");

	std::string FolderName = "Log\\LogFile\\" + tmToString(localTime, "%Y_%m%d_Log");

	std::filesystem::path currentPath = std::filesystem::current_path();	// 현재 경로
	std::filesystem::create_directory(currentPath / "Log");					// 새로운 폴더 생성 (만약 이미 존재하면 무시됨)
	std::filesystem::create_directory(currentPath / "Log\\LogFile");		// 새로운 폴더 생성 (만약 이미 존재하면 무시됨)
	std::filesystem::create_directory(currentPath / FolderName);			// 새로운 폴더 생성 (만약 이미 존재하면 무시됨)
	std::filesystem::path filePath = currentPath / FolderName / FileName;	// 파일 경로 및 이름 
	mLogWriteFile.open(filePath, std::ios::out | std::ios::trunc);

	Sample();

	return mLogWriteFile.is_open();
}


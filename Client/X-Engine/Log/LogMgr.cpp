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

	freopen_s(&mConsoleStream, "CONOUT$", "w", stdout); // ǥ�� ����� �ַܼ� ���𷺼�
	freopen_s(&mConsoleStream, "CONIN$", "r", stdin);	// ǥ�� �Է��� �ַܼ� ���𷺼�
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
	LOG_CONSOLE(LogLevel::Info, "Sample() �Լ����� �α� ��� �����ڵ带 �ۼ��߽��ϴ�.\n\n");
		
	LOG_CONSOLE(LogLevel::Basic, "�ܼ�â���� �α׸� ����ϴ�.\n");
	
	COUT("�α� ���� Basic �̸� �ܼ�â���� �α� ����ϴ�.\n");
	
	LOG_CONSOLE(LogLevel::Debug,	"Magenta �α�\n");
	LOG_CONSOLE(LogLevel::Error,	"Red �α�\n");
	LOG_CONSOLE(LogLevel::Fatal,	"Green �α�\n");
	LOG_CONSOLE(LogLevel::Info,		"White �α�\n");
	LOG_CONSOLE(LogLevel::Trace,	"Cyan �α�\n");
	LOG_CONSOLE(LogLevel::Warning,	"Yellow �α�\n");

	LOG_FILE(LogLevel::Debug, "���Ͽ��� �α� �ۼ�\n");
	LOG_CONSOLE_FILE(LogLevel::Debug, "�ܼ�â �� ���� �Ѵ� �α� �ۼ�\n");

}

void LogMgr::LogConsole(LogLevel level, const char* file, const char* func, const int& line, std::string logstr)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	std::string result{};
	std::string logLv = "[ " + LogLevelString(level) + " ]\t";
	std::string filepath{ file };
	std::string filename = LOG_MGR->GetCurFileName(filepath);

	// [ �α�Ÿ�� ] ����::�Լ���(����) : ����
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

	SetConsoleTextAttribute(hConsole, (DWORD)ConsoleColor::White); // �⺻ �������� ����
}

void LogMgr::LogFile(LogLevel level, const char* file, const char* func, const int& line, std::string logstr)
{
	if (mLogWriteFile)
	{
		std::string result{};
		std::string logLv = "[ " + LogLevelString(level) + " ]\t";
		std::string filepath{ file };
		std::string filename = LOG_MGR->GetCurFileName(filepath);


		// [ �α�Ÿ�� ] ����::�Լ���(����) : ����
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

	std::filesystem::path currentPath = std::filesystem::current_path();	// ���� ���
	std::filesystem::create_directory(currentPath / "Log");					// ���ο� ���� ���� (���� �̹� �����ϸ� ���õ�)
	std::filesystem::create_directory(currentPath / "Log\\LogFile");		// ���ο� ���� ���� (���� �̹� �����ϸ� ���õ�)
	std::filesystem::create_directory(currentPath / FolderName);			// ���ο� ���� ���� (���� �̹� �����ϸ� ���õ�)
	std::filesystem::path filePath = currentPath / FolderName / FileName;	// ���� ��� �� �̸� 
	mLogWriteFile.open(filePath, std::ios::out | std::ios::trunc);

	Sample();

	return mLogWriteFile.is_open();
}


#include "EnginePch.h"
#include "LogMgr.h"

#include <iostream>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <filesystem>

LogManager::LogManager()
{

}

LogManager::~LogManager()
{
	if (mConsoleStream) {
		FreeConsole();

		if (mIsRead)
			return;

		if (mLogWriteFile.is_open()) {
			mLogWriteFile.close();
		}
	}
}


void LogManager::LogConsole(LogLevel level, const char* file, const char* func, const int& line, std::string logstr)
{
	if (mIsConsoleLogWrite == false)
		return;

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	std::string result{};
	std::string logLv = "[ " + LogLevelString(level) + " ]\t";
	std::string filepath{ file };
	std::string filename = LOG_MGR->GetCurFileName(filepath);

	// [ �α�Ÿ�� ] ����::�Լ���(����) : ����
	result = logLv + filename + "::" + func + "(" + std::to_string(line) + ") : " + logstr;

	switch (level)
	{
	case LogLevel::Trace:
	{
		SetConsoleTextAttribute(mConsoleHandle, (WORD)TextColor::Cyan);
		std::cout << result;
	}
	break;
	case LogLevel::Debug:
	{
		SetConsoleTextAttribute(mConsoleHandle, (WORD)TextColor::Magenta);
		std::cout << result;
	}
	break;
	case LogLevel::Info:
	{
		SetConsoleTextAttribute(mConsoleHandle, (WORD)TextColor::White);
		std::cout << result;
	}
	break;
	case LogLevel::Warning:
	{
		SetConsoleTextAttribute(mConsoleHandle, (WORD)TextColor::Yellow);
		std::cout << result;
	}
	break;
	case LogLevel::Error:
	{
		SetConsoleTextAttribute(mConsoleHandle, (WORD)TextColor::Red);
		std::cout << result;
	}
	break;
	case LogLevel::Fatal:
	{
		SetConsoleTextAttribute(mConsoleHandle, (WORD)TextColor::Green);
		std::cout << result;
	}
	break;
	}

	SetConsoleTextAttribute(mConsoleHandle, (WORD)TextColor::White); // �⺻ �������� ����
}

void LogManager::LogFile(LogLevel level, const char* file, const char* func, const int& line, std::string logstr)
{
	if (mIsFileLogWrite == false)
		return;

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


bool LogManager::Init(const std::string& name)
{
	/// +-----------------------------
	///		   CREATE CONSOLE
	/// -----------------------------+
	///
	AllocConsole();

	freopen_s(&mConsoleStream, "CONOUT$", "w", stdout); // ǥ�� ����� �ַܼ� ���𷺼�
	freopen_s(&mConsoleStream, "CONIN$", "r", stdin);	// ǥ�� �Է��� �ַܼ� ���𷺼�
	mConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	/// +-----------------------------
	///		   CREATE LOG FILE
	/// -----------------------------+
	///
	if (name != "") {
		std::time_t currentTime = std::time(nullptr);
		std::tm		localTime{};
		localtime_s(&localTime, &currentTime);

		std::string FileName = name + " " + tmToString(localTime, "%Y_%m%d_%Hh%Mm%Ss.log");
		std::string FolderName = "Log\\LogFile\\" + tmToString(localTime, "%Y_%m%d_Log");

		std::filesystem::path currentPath = std::filesystem::current_path();	// ���� ���
		std::filesystem::create_directory(currentPath / "Log");					// ���ο� ���� ���� (���� �̹� �����ϸ� ���õ�)
		std::filesystem::create_directory(currentPath / "Log\\LogFile");		// ���ο� ���� ���� (���� �̹� �����ϸ� ���õ�)
		std::filesystem::create_directory(currentPath / FolderName);			// ���ο� ���� ���� (���� �̹� �����ϸ� ���õ�)
		std::filesystem::path filePath = currentPath / FolderName / FileName;	// ���� ��� �� �̸� 
		mLogWriteFile.open(filePath, std::ios::out | std::ios::trunc);
		mIsRead = true;
	}


	/// +-----------------------------
	///		INIT WCOUT LOCALE KOREA
	/// -----------------------------+
	std::wcout.imbue(std::locale("kor"));

	return mLogWriteFile.is_open();
}

void LogManager::On()
{
	mIsConsoleLogWrite = true;
	mIsFileLogWrite = true;
}

void LogManager::Off()
{
	mIsConsoleLogWrite = false;
	mIsFileLogWrite = false;

}

void LogManager::On_ConsoleLog()
{
	mIsConsoleLogWrite = true;

}

void LogManager::Off_ConsoleLog()
{
	mIsConsoleLogWrite = false;

}

void LogManager::On_FileLog()
{
	mIsFileLogWrite = true;

}

void LogManager::Off_FileLog()
{
	mIsFileLogWrite = false;

}

std::string LogManager::tmToString(const std::tm& timeStruct, const std::string& format)
{
	std::stringstream ss;
	ss << std::put_time(&timeStruct, format.c_str());
	return ss.str();
}


std::string LogManager::LogLevelString(LogLevel level)
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
	}
	return "";
}

std::string LogManager::GetCurFileName(std::string& src_File)
{
	if (src_File.size() == 0) return "";

	std::string name{};
	for (size_t i = src_File.size() - 1; src_File[i] != '\\'; --i)
	{
		name += src_File[i];
	}
	std::reverse(name.begin(), name.end());
	return name;
}

std::string LogManager::CharToString(char* c)
{
	std::string str = c;
	return str;
}

std::string LogManager::WstringToString(const WCHAR* wc)
{
	std::wstring wstr = wc;

	// wstring�� string���� ��ȯ�ϴ� �κ�
	std::string str;
	str.reserve(wstr.size());  // ������ ���� �̸� �޸𸮸� �Ҵ�

	// �������� ���� ��� for ������ ����Ͽ� ��ȯ
	std::ranges::transform(wstr, std::back_inserter(str), [](wchar_t wc) {
		return static_cast<char>(wc);
		});
	return str;
}

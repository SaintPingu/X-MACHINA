#include "pch.h"
#include "LogManager.h"

#include <istream>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <filesystem>

DEFINE_SINGLETON(LogManager);

LogManager::LogManager()
{
	AllocConsole();

	freopen_s(&mConsoleStream, "CONOUT$", "w", stdout); // 표준 출력을 콘솔로 리디렉션
	freopen_s(&mConsoleStream, "CONIN$", "r", stdin);	// 표준 입력을 콘솔로 리디렉션

}

LogManager::~LogManager()
{

	const int bufferSize = 100; // 버퍼 크기 정의
	char buffer[bufferSize];    // 입력을 저장할 배열

	std::cout << "종료하려면 아무 키나 입력하세요 : ";
	std::cin.getline(buffer, bufferSize); // 사용자로부터 문자열 입력 받음

	FreeConsole();
	if (mLogWriteFile.is_open())
		mLogWriteFile.close();

	fclose(mConsoleStream);

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

	// [ 로그타입 ] 파일::함수명(라인) : 내용
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

	SetConsoleTextAttribute(mConsoleHandle, (WORD)TextColor::White); // 기본 색상으로 복원
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


bool LogManager::Init(std::string name)
{
	/// +-----------------------------
	///		   CREATE LOG FILE
	/// -----------------------------+
	if (mIsFileLogWrite == true) {
		std::time_t currentTime = std::time(nullptr);
		std::tm		localTime{};
		localtime_s(&localTime, &currentTime);

		std::string FileName = name + " " + tmToString(localTime, "%Y_%m%d_%Hh%Mm%Ss.log");
		std::string FolderName = "Log\\LogFile\\" + tmToString(localTime, "%Y_%m%d_Log");

		std::filesystem::path currentPath = std::filesystem::current_path();	// 현재 경로
		std::filesystem::create_directory(currentPath / "Log");					// 새로운 폴더 생성 (만약 이미 존재하면 무시됨)
		std::filesystem::create_directory(currentPath / "Log\\LogFile");		// 새로운 폴더 생성 (만약 이미 존재하면 무시됨)
		std::filesystem::create_directory(currentPath / FolderName);			// 새로운 폴더 생성 (만약 이미 존재하면 무시됨)
		std::filesystem::path filePath = currentPath / FolderName / FileName;	// 파일 경로 및 이름 
		mLogWriteFile.open(filePath, std::ios::out | std::ios::trunc);

	}

	mConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	/// +-----------------------------
	///		INIT WCOUT LOCALE KOREA
	/// -----------------------------+
	std::wcout.imbue(std::locale("kor"));

	return mConsoleHandle;
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

	// wstring을 string으로 변환하는 부분
	std::string str;
	str.reserve(wstr.size());  // 성능을 위해 미리 메모리를 할당

	// 레인지와 범위 기반 for 루프를 사용하여 변환
	std::ranges::transform(wstr, std::back_inserter(str), [](wchar_t wc) {
		return static_cast<char>(wc);
		});
	return str;
}

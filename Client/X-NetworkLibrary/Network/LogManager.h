#pragma once
#include <fstream>
#include "UtilityMacro.h"


#define CHAR_TO_STRING(c) LogManager::GetInst()->CharToString(c)
#define WSTR_TO_STRING(str) LogManager::GetInst()->WstringToString(str)

enum class TextColor {
	Default = 0x07,    // White

	Black         = 0x00,
	Red           = 0x04,
	Green         = 0x02,
	Yellow        = 0x06,
	Blue          = 0x01,
	Magenta       = 0x05,
	Cyan          = 0x03,
	White         = 0x07,
	BrightBlack   = 0x08,
	BrightRed     = 0x0C,
	BrightGreen   = 0x0A,
	BrightYellow  = 0x0E,
	BrightBlue    = 0x09,
	BrightMagenta = 0x0D,
	BrightCyan    = 0x0B,
	BrightWhite   = 0x0F,
	ExtraBlack    = 0x10,
	ExtraRed      = 0x14,
	ExtraGreen    = 0x12,
	ExtraYellow   = 0x16,
	ExtraBlue     = 0x11,
	ExtraMagenta  = 0x15,
	ExtraCyan     = 0x13,
	ExtraWhite    = 0x17
};

enum class LogLevel
{
	Trace,
	Debug,
	Info,
	Warning,
	Error,
	Fatal,

	END,
};

#define LOG_MGR LogManager::GetInst()
class LogManager
{
	DECLARE_SINGLETON(LogManager);

private:
	HANDLE			mConsoleHandle{};
	FILE*			mConsoleStream{};	/// Console 

	bool			mIsConsoleLogWrite = true;
	bool			mIsFileLogWrite    = false;

	std::ofstream	mLogWriteFile;	/// 파일 : 저장경로- LogFile/Year_MonthDay/~~~.log ( txt 파일이다. )

public:
	LogManager();
	~LogManager();

public:
	template<typename... Args>
	void Cout(Args... args);

	template<typename... Args>
	void WCout(Args... args);

	template<typename... Args>
	void File(Args... args);

	// 함수를 통해 ANSI Escape Code를 이용하여 텍스트 색상 변경
	template<typename T>
	void SetColor(T color);


public:
	bool Init(std::string name = "");

	/// +-----------------------------
	///			   ON / OFF
	/// -----------------------------+
	void On();
	void Off();

	void On_ConsoleLog();
	void Off_ConsoleLog();

	void On_FileLog();
	void Off_FileLog();

	void LogConsole(LogLevel level, const char* file, const char* func, const int& line, std::string logstr);		/// 콘솔창에 로그를 띄웁니다.
	void LogFile(LogLevel level, const char* file, const char* func, const int& line, std::string logstr);			/// 파일에   로그를 기록합니다.


	/// +-----------------------------
	///			Log Utility
	/// -----------------------------+
	std::string tmToString(const std::tm& timeStruct, const std::string& format); /// tm 을 string 으로 변환합니다. 
	std::string LogLevelString(LogLevel level);									  /// LogLevel 을 string 으로 변환합니다. 
	std::string GetCurFileName(std::string& src_File);
	std::string CharToString(char* c);
	std::string WstringToString(const WCHAR* wc);


};

template<typename ...Args>
inline void LogManager::Cout(Args... args)
{
	if (mIsConsoleLogWrite == false)
		return;

	((std::cout << args), ...);
}

template<typename ...Args>
inline void LogManager::WCout(Args... args)
{
	if (mIsConsoleLogWrite == false)
		return;

	((std::wcout << args), ...);
}

template<typename ...Args>
inline void LogManager::File(Args ...args)
{
	if (mIsFileLogWrite == false)
		return;

	((mLogWriteFile << args), ...);

}

template<typename T>
inline void LogManager::SetColor(T color)
{
	SetConsoleTextAttribute(mConsoleHandle, static_cast<WORD>(color));
}
#pragma once

#define LOG_MGR								LogMgr::Inst()
#define LOG_CONSOLE(LogLevel, LogString)	LogMgr::Inst()->LogConsole(LogLevel, __FILE__, __func__, __LINE__, LogString)
#define LOG_FILE(LogLevel, LogString)		LogMgr::Inst()->LogFile(LogLevel, __FILE__, __func__, __LINE__, LogString);
#define LOG_CONSOLE_FILE(LogLevel, LogString) \
											LogMgr::Inst()->LogConsole(LogLevel, __FILE__, __func__, __LINE__, LogString);\
											LogMgr::Inst()->LogFile(LogLevel, __FILE__, __func__, __LINE__, LogString);
#define COUT(logString)						LogMgr::Inst()->LogConsole(LogLevel::Basic, __FILE__, __func__, __LINE__, logString)

enum class ConsoleColor
{
	Black   = 0,
	Blue    = FOREGROUND_BLUE,
	Green   = FOREGROUND_GREEN,
	Cyan    = FOREGROUND_GREEN | FOREGROUND_BLUE,
	Red     = FOREGROUND_RED,
	Magenta = FOREGROUND_RED | FOREGROUND_BLUE,
	Yellow  = FOREGROUND_RED | FOREGROUND_GREEN,
	White   = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
};

enum class LogLevel
{
	Basic,
	Trace,
	Debug,
	Info,
	Warning,
	Error,
	Fatal,
};

class LogMgr : public Singleton<LogMgr> {
	friend class Singleton<LogMgr>;

private:
	FILE* mConsoleStream{};								/// Console 
	bool			mIsLogWrite{};

	std::ofstream	mLogWriteFile{};								/// 파일 : 저장경로- LogFile/Year_MonthDay/~~~.log ( txt 파일이다. )

public:
	LogMgr();
	virtual ~LogMgr();

public:
	bool Init(std::string name = "");

	void SetLogWrite(bool logstatus) { mIsLogWrite = logstatus; }
	bool GetIsLogWrite() { return mIsLogWrite; }

	// 콘솔창에 로그를 띄웁니다.
	void LogConsole(LogLevel level, const char* file, const char* func, const int& line, std::string logstr);
	// 파일에 로그를 기록합니다.
	void LogFile(LogLevel level, const char* file, const char* func, const int& line, std::string logstr);


	std::string tmToString(const std::tm& timeStruct, const std::string& format); // tm 을 string 으로 변환합니다. 
	std::string LogLevelString(LogLevel level);									  // LogLevel 을 string 으로 변환합니다. 
	std::string GetCurFileName(std::string& src_File);

public:
	void Sample();

};
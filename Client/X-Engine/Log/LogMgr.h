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

	std::ofstream	mLogWriteFile{};								/// ���� : ������- LogFile/Year_MonthDay/~~~.log ( txt �����̴�. )

public:
	LogMgr();
	virtual ~LogMgr();

public:
	bool Init(std::string name = "");

	void SetLogWrite(bool logstatus) { mIsLogWrite = logstatus; }
	bool GetIsLogWrite() { return mIsLogWrite; }

	// �ܼ�â�� �α׸� ���ϴ�.
	void LogConsole(LogLevel level, const char* file, const char* func, const int& line, std::string logstr);
	// ���Ͽ� �α׸� ����մϴ�.
	void LogFile(LogLevel level, const char* file, const char* func, const int& line, std::string logstr);


	std::string tmToString(const std::tm& timeStruct, const std::string& format); // tm �� string ���� ��ȯ�մϴ�. 
	std::string LogLevelString(LogLevel level);									  // LogLevel �� string ���� ��ȯ�մϴ�. 
	std::string GetCurFileName(std::string& src_File);

public:
	void Sample();

};
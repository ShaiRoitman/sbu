#pragma once

#include <string>
#include <memory>
#include <list>

class LoggingComponentEntry
{
public:
	std::string componentName;
	std::string level;
};

class LoggingOptions
{
public:
	LoggingOptions();
	std::list<LoggingComponentEntry> components;
	bool shouldLogToConsole;
	std::string fileOutputName;
	std::string rootLevel;
};

class ILogger
{
public:
	virtual void TraceFormat(const char* format, ...);
	virtual void DebugFormat(const char* format, ...);
	virtual void InfoFormat(const char* format, ...);
	virtual void WarningFormat(const char* format, ...);
	virtual void ErrorFormat(const char* format, ...);
	virtual void FatalFormat(const char* format, ...);


	virtual void Trace(const char* str) = 0;
	virtual void Debug(const char* str) = 0;
	virtual void Info(const char* str) = 0;
	virtual void Warning(const char* str) = 0;
	virtual void Error(const char* str) = 0;
	virtual void Fatal(const char* str) = 0;
};

class LoggerFactory
{
public:
	static void InitLogger(LoggingOptions& loggingComponents);
	static std::shared_ptr<ILogger> getLogger(const char* component);
};

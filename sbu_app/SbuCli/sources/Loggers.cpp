#include "Loggers.h"
#include "stdarg.h"
#include "StandardOutputWrapper.h"

#include "Poco/ConsoleChannel.h"
#include "Poco/StreamChannel.h"
#include "Poco/SplitterChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/PatternFormatter.h"
#include "Poco/FileChannel.h"
#include "Poco/AutoPtr.h"
#include "Poco/Logger.h"
#include <iostream>

const int bufferSize = 64 * 1024;

LoggingOptions::LoggingOptions()
{
	this->shouldLogToConsole = false;
	this->rootLevel = "Information";
	this->fileOutputName = "";
}

class InitRootLogger
{
public:
	InitRootLogger()
	{
		sChannel = new Poco::SplitterChannel();
		Poco::AutoPtr<Poco::PatternFormatter> pPF(new Poco::PatternFormatter());
		pPF->setProperty("pattern", "%Y-%m-%d %H:%M:%S [%P:%I] [%s]:[%p]: %t");
		Poco::AutoPtr<Poco::FormattingChannel> pFC(new Poco::FormattingChannel(pPF, sChannel));

		Poco::Logger::root().setChannel(pFC);
		Poco::Logger::root().setLevel("information");
	}

	static Poco::AutoPtr<Poco::SplitterChannel> sChannel;
};

Poco::AutoPtr<Poco::SplitterChannel> InitRootLogger::sChannel;
static InitRootLogger initLogger;

void ILogger::TraceFormat(const char* format, ...)
{
	char buffer[bufferSize];
	va_list arglist;
	va_start(arglist, format);
	vsprintf(buffer, format, arglist);
	va_end(arglist);
	this->Trace(buffer);
}

void ILogger::DebugFormat(const char* format, ...)
{
	char buffer[bufferSize];
	va_list arglist;
	va_start(arglist, format);
	vsprintf(buffer, format, arglist);
	va_end(arglist);
	this->Debug(buffer);
}
void ILogger::InfoFormat(const char* format, ...)
{
	char buffer[bufferSize];
	va_list arglist;
	va_start(arglist, format);
	vsprintf(buffer, format, arglist);
	va_end(arglist);
	this->Info(buffer);
}

void ILogger::WarningFormat(const char* format, ...)
{
	char buffer[bufferSize];
	va_list arglist;
	va_start(arglist, format);
	vsprintf(buffer, format, arglist);
	va_end(arglist);
	this->Warning(buffer);
}


void ILogger::ErrorFormat(const char* format, ...)
{
	char buffer[bufferSize];
	va_list arglist;
	va_start(arglist, format);
	vsprintf(buffer, format, arglist);
	va_end(arglist);
	this->Error(buffer);
}

void ILogger::FatalFormat(const char* format, ...)
{
	char buffer[bufferSize];
	va_list arglist;
	va_start(arglist, format);
	vsprintf(buffer, format, arglist);
	va_end(arglist);
	this->Fatal(buffer);
}

class Logger : public ILogger
{
public:
	Logger(const char* component) :
		m_Logger(Poco::Logger::get(component))
	{
	}

	virtual void Trace(const char * str) override
	{
		m_Logger.trace(str);
	}

	virtual void Debug(const char * str) override
	{
		m_Logger.debug(str);
	}

	virtual void Info(const char * str) override
	{
		m_Logger.information(str);
	}

	virtual void Warning(const char * str) override
	{
		m_Logger.warning(str);
	}

	virtual void Error(const char * str) override
	{
		m_Logger.error(str);
	}

	virtual void Fatal(const char * str) override
	{
		m_Logger.fatal(str);

	}
protected:
	Poco::Logger& m_Logger;
};

void LoggerFactory::InitLogger(LoggingOptions& loggingComponents)
{
	if (loggingComponents.shouldLogToConsole)
	{
		Poco::AutoPtr<Poco::StreamChannel> cChannel(new Poco::StreamChannel(std::cout));
		InitRootLogger::sChannel->addChannel(cChannel);
	}

	if (loggingComponents.fileOutputName.size() != 0)
	{
		Poco::AutoPtr<Poco::FileChannel> pChannel(new Poco::FileChannel());
		pChannel->setProperty("path", loggingComponents.fileOutputName);
		InitRootLogger::sChannel->addChannel(pChannel);
	}

	Poco::Logger::setLevel(Poco::Logger::ROOT, Poco::Logger::parseLevel(loggingComponents.rootLevel));

	for (auto iter = loggingComponents.components.begin(); iter != loggingComponents.components.end(); ++iter)
	{
		try
		{
			auto level = Poco::Logger::parseLevel(iter->level);
			Poco::Logger::setLevel(iter->componentName, level);
		}
		catch (std::exception ex)
		{
			StandardOutputWrapper::GetInstance()->OutputLine(std::string("Failed to parse logging ") + ex.what());
		}
	}
}

std::shared_ptr<ILogger> LoggerFactory::getLogger(const char* component)
{
	return std::make_shared<Logger>(component);
}

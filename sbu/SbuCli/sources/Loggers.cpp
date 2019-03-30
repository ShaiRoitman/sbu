#include "Loggers.h"
#include "stdarg.h"

#include "Poco/ConsoleChannel.h"
#include "Poco/StreamChannel.h"
#include "Poco/SplitterChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/PatternFormatter.h"
#include "Poco/FileChannel.h"
#include "Poco/AutoPtr.h"
#include "Poco/Logger.h"

const int bufferSize = 64 * 1024;

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

void LoggerFactory::InitLogger(boost::program_options::variables_map& vm)
{
	Poco::AutoPtr<Poco::SplitterChannel> sChannel(new Poco::SplitterChannel());
	Poco::AutoPtr<Poco::PatternFormatter> pPF(new Poco::PatternFormatter());	pPF->setProperty("pattern", "%Y-%m-%d %H:%M:%S [%P:%I] [%s]:[%p]: %t");	Poco::AutoPtr<Poco::FormattingChannel> pFC(new Poco::FormattingChannel(pPF, sChannel));
	std::string logFormat = "[%TimeStamp%]: [%Severity%] [%Message%]";

	if (!vm["Logging.Console"].empty())
	{
		auto value = vm["Logging.Console"].as<std::string>();
		if (vm["Logging.Console"].as<std::string>() == "true")
		{
			Poco::AutoPtr<Poco::StreamChannel> cChannel(new Poco::StreamChannel(std::cout));
			sChannel->addChannel(cChannel);
		}
	}

	if (!vm["Logging.FileOutput"].empty())
	{
		auto fileName = vm["Logging.FileOutput"].as<std::string>();
		if (fileName != "")
		{
			Poco::AutoPtr<Poco::FileChannel> pChannel(new Poco::FileChannel());
			pChannel->setProperty("path", fileName);
			sChannel->addChannel(pChannel);
		}
	}

	Poco::Logger::root().setChannel(pFC);
	Poco::Logger::root().setLevel("information");
}

std::shared_ptr<ILogger> LoggerFactory::getLogger(const char* component)
{
	return std::make_shared<Logger>(component);
}

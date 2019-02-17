#include "Loggers.h"
#include "boost/log/utility/setup/console.hpp"
#include "boost/log/utility/setup/file.hpp"
#include "boost/log/sources/severity_channel_logger.hpp"
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>

void ILogger::TraceFormat(const char* format, ...)
{
	char buffer[64 * 1024];
	va_list arglist;
	va_start(arglist, format);
	vsprintf(buffer, format, arglist);
	va_end(arglist);
	this->Trace(buffer);
}

void ILogger::DebugFormat(const char* format, ...)
{
	char buffer[64 * 1024];
	va_list arglist;
	va_start(arglist, format);
	vsprintf(buffer, format, arglist);
	va_end(arglist);
	this->Debug(buffer);
}
void ILogger::InfoFormat(const char* format, ...)
{
	char buffer[64 * 1024];
	va_list arglist;
	va_start(arglist, format);
	vsprintf(buffer, format, arglist);
	va_end(arglist);
	this->Info(buffer);
}

void ILogger::WarningFormat(const char* format, ...)
{
	char buffer[64 * 1024];
	va_list arglist;
	va_start(arglist, format);
	vsprintf(buffer, format, arglist);
	va_end(arglist);
	this->Warning(buffer);
}


void ILogger::ErrorFormat(const char* format, ...)
{
	char buffer[64 * 1024];
	va_list arglist;
	va_start(arglist, format);
	vsprintf(buffer, format, arglist);
	va_end(arglist);
	this->Error(buffer);
}

void ILogger::FatalFormat(const char* format, ...)
{
	char buffer[64 * 1024];
	va_list arglist;
	va_start(arglist, format);
	vsprintf(buffer, format, arglist);
	va_end(arglist);
	this->Fatal(buffer);
}


class Logger : public ILogger
{
public:
	Logger(const char* component)
		: m_Logger(boost::log::keywords::channel = (component))
	{
	}

	virtual void Trace(const char * str) override
	{
		BOOST_LOG_SEV(m_Logger, boost::log::trivial::trace) << str;
	}

	virtual void Debug(const char * str) override
	{
		BOOST_LOG_SEV(m_Logger, boost::log::trivial::debug) << str;
	}

	virtual void Info(const char * str) override
	{
		BOOST_LOG_SEV(m_Logger, boost::log::trivial::info) << str;
	}

	virtual void Warning(const char * str) override
	{
		BOOST_LOG_SEV(m_Logger, boost::log::trivial::warning) << str;
	}

	virtual void Error(const char * str) override
	{
		BOOST_LOG_SEV(m_Logger, boost::log::trivial::error) << str;
	}

	virtual void Fatal(const char * str) override
	{
		BOOST_LOG_SEV(m_Logger, boost::log::trivial::fatal) << str;
	}
protected:
	boost::log::sources::severity_channel_logger_mt<boost::log::trivial::severity_level> m_Logger;
};

void LoggerFactory::InitLogger(boost::program_options::variables_map& vm)
{
	std::string logFormat = "[%TimeStamp%]: [%Severity%] [%Message%]";

	boost::log::core::get()->set_logging_enabled(false);
	boost::log::core::get()->add_global_attribute("TimeStamp", boost::log::attributes::local_clock());

	if (!vm["Logging.Console"].empty())
	{
		auto value = vm["Logging.Console"].as<std::string>();
		if (vm["Logging.Console"].as<std::string>() == "true")
		{
			boost::log::add_console_log(std::cout, boost::log::keywords::format = logFormat);
			boost::log::core::get()->set_logging_enabled(true);
		}
	}
	if (!vm["Logging.FileOutput"].empty())
	{
		auto fileName = vm["Logging.FileOutput"].as<std::string>();
		if (fileName != "")
		{
			boost::log::add_file_log(fileName, boost::log::keywords::format = logFormat);
			boost::log::core::get()->set_logging_enabled(true);
		}
	}
}

std::shared_ptr<ILogger> LoggerFactory::getLogger(const char* component)
{
	return std::make_shared<Logger>(component);
}

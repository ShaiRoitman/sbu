#include "Loggers.h"
#include "boost/log/utility/setup/console.hpp"
#include "boost/log/utility/setup/file.hpp"
#include "boost/log/sources/severity_channel_logger.hpp"
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions/keyword.hpp>

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
	if (!vm["Logging.Console"].empty())
	{
		if (vm["Logging.Console"].as<std::string>() == "true")
		{
			boost::log::add_console_log(std::cout, boost::log::keywords::format = logFormat);
		}
	}
	if (!vm["Logging.FileOutput"].empty())
	{
		auto fileName = vm["Logging.FileOutput"].as<std::string>();
		if (fileName != "")
		{
			boost::log::add_file_log(fileName, boost::log::keywords::format = logFormat);
		}
	}

	auto logger = getLogger("application");
	logger->Info("Init Application");
}

std::shared_ptr<ILogger> LoggerFactory::getLogger(const char* component)
{
	return std::make_shared<Logger>(component);
}

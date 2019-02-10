#pragma once

#include <string>
#include <memory>
#include <boost/program_options/variables_map.hpp>

class ILogger
{
public:
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
	static void InitLogger(boost::program_options::variables_map& vm);
	static std::shared_ptr<ILogger> getLogger(const char* component);
};

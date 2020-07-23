#pragma once

#include "boost/program_options.hpp"
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
	std::list<LoggingComponentEntry> components;
};

class CommandLineAndOptions
{
public:
	int ParseOptions(int argc, const char* argv[], LoggingOptions& loggingOptions);

	boost::program_options::variables_map vm;
};
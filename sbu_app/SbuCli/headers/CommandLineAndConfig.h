#pragma once

#include "boost/program_options.hpp"
#include "Loggers.h"
#include <list>

class CommandLineAndOptions
{
public:
	int ParseOptions(int argc, const char* argv[]);

	LoggingOptions loggingOptions;
	boost::program_options::variables_map vm;
};
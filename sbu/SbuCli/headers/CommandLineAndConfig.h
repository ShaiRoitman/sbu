#pragma once

#include "boost/program_options.hpp"

class CommandLineAndOptions
{
public:
	int ParseOptions(int argc, const char* argv[]);

	boost::program_options::variables_map vm;
};
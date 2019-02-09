#include "CommandLineAndConfig.h"
#include "boost/filesystem.hpp"
#include <iostream>

using namespace boost::program_options;

const std::string g_version = "0.9";
const std::string g_developer_name = "Shai Roitman";

int CommandLineAndOptions::ParseOptions(int argc, const char* argv[])
{
	const std::string configFileEnvVar = "SBU_CONFIG";
	options_description desc("Allowed options");

	std::string path;
	std::string name;
	std::string action;
	std::string date;

	desc.add_options()
		("help,h", "print usage message")
		("version", "print version")
		("action,a", value(&action), "Actions include : CreateBackupDef, ListBackupDef, Backup, Restore, ListBackup")
		("path,p", value(&path), "Path")
		("name,n", value(&name), "Name")
		("date,d", value(&date), "The last effective date - Defaults to now()")
		;

	try 
	{
		store(parse_command_line(argc, argv, desc), vm);
		std::string sbuConfigFileName = "sbu.config";

		if (std::getenv(configFileEnvVar.c_str()))
		{
			sbuConfigFileName = std::getenv(configFileEnvVar.c_str());
		}
		if (boost::filesystem::exists(sbuConfigFileName))
		{
			store(parse_config_file<char>(sbuConfigFileName.c_str(), desc, false), vm);
		}

		store(parse_environment(desc,
			[](const std::string& i_env_var)
		{// maps environment variable "HOSTNAME" to user-defined option "hostname"
			return i_env_var == "HOSTNAME" ? "hostname" : "";
		}),
			vm);

		if (!vm["help"].empty())
		{
			std::cout << std::string("sbu ( Smart Backup Utility ) : ") + g_version + std::string("\n");
			std::cout << std::string("Written by ") << std::string(g_developer_name) + std::string("\n\n");
			std::cout << desc << std::string("\n");
			return 0;
		}

		if (!vm["version"].empty())
		{
			std::cout << std::string("sbu ( Smart Backup Utility ) : ") + g_version + std::string("\n");
			std::cout << std::string("Written by ") << std::string(g_developer_name) + std::string("\n\n");
			return 0;
		}
	}
	catch (const boost::program_options::unknown_option exception)
	{
		std::cout << std::string("Invalid option ") << exception.get_option_name() << std::string("\n");
		return 1;
	}

	return 0;
}
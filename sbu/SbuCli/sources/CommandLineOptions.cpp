#include "CommandLineAndConfig.h"
#include "boost/filesystem.hpp"
#include <iostream>
#include "ExitCodes.h"

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
	std::string config_file;
	std::string logging;

	desc.add_options()
		("help,h", "print usage message")
		("version", "print version")
		("config", value(&config_file), "Config file")
		("action,a", value(&action), "Actions include : CreateBackupDef, ListBackupDef, Backup, Restore, ListBackup")
		("path,p", value(&path), "Path")
		("name,n", value(&name), "Name")
		("date,d", value(&date), "The last effective date - Defaults to now()")
		("showOnly", "In restore show only the files to be restored")
		("FileRepository.name", "The database name of the FileRepository")
		("FileRepository.path", "Path of the FileRepository")
		("Logging.Console", value(&logging), "true/false - Enable logs to console")
		("Logging.FileOutput", value(&logging), "filename - if exists emit logs to the file")
		;

	try 
	{
		store(parse_command_line(argc, argv, desc), vm);
		std::string sbuConfigFileName;

		char* configFile = std::getenv(configFileEnvVar.c_str());
		if (configFile != NULL)
		{
			sbuConfigFileName = configFile;
		}

		if (!vm["config"].empty())
		{
			sbuConfigFileName = vm["config"].as<std::string>();
		}

		if (!sbuConfigFileName.empty())
		{
			if (boost::filesystem::exists(sbuConfigFileName))
			{
				store(parse_config_file<char>(sbuConfigFileName.c_str(), desc, true), vm);
			}
			else
			{
				return ExitCode_ConfigFileMissing;
			}
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
			return ExitCode_HelpCalled;
		}

		if (!vm["version"].empty())
		{
			std::cout << std::string("sbu ( Smart Backup Utility ) : ") + g_version + std::string("\n");
			std::cout << std::string("Written by ") << std::string(g_developer_name) + std::string("\n\n");
			return ExitCode_VersionCalled;
		}

		if (vm["action"].empty())
		{
			std::cout << "Missing action argument\n";
			return ExitCode_MissingAction;
		}
	}
	catch (const boost::program_options::unknown_option exception)
	{
		std::cout << std::string("Invalid option ") << exception.get_option_name() << std::string("\n");
		return ExitCode_InvalidArgument;
	}

	return ExitCode_Success;
}
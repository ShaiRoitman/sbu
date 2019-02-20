#include "CommandLineAndConfig.h"
#include "boost/filesystem.hpp"
#include <iostream>
#include "ExitCodes.h"
#include "Loggers.h"

using namespace boost::program_options;

const std::string g_version = "0.9";
const std::string g_developer_name = "Shai Roitman";

int CommandLineAndOptions::ParseOptions(int argc, const char* argv[])
{
	int retValue = ExitCode_Success;

	const std::string configFileEnvVar = "SBU_CONFIG";
	options_description desc("Allowed options");

	std::string path;
	std::string name;
	std::string action;
	std::string date;
	std::string config_file;
	std::string logging;
	std::string logging_verbosity;
	std::string workdir;
	std::string filerepName;
	std::string filerepPath;
	std::string repPath;

	desc.add_options()
		("help,h", "print usage message")
		("version", "print version")
		("config", value(&config_file), "Config file")
		("action,a", value(&action), "Actions include : CreateBackupDef, ListBackupDef, Backup, Restore, ListBackup")
		("path,p", value(&path), "Path")
		("name,n", value(&name), "Name")
		("date,d", value(&date), "The last effective date - Defaults to now()")
		("showOnly", "In restore show only the files to be restored")
		("FileRepository.name", value(&filerepName), "The database name of the FileRepository")
		("FileRepository.path", value(&filerepPath), "Path of the FileRepository")
		("Logging.Console", value(&logging), "true/false - Enable logs to console")
		("Logging.FileOutput", value(&logging), "filename - if exists emit logs to the file")
		("Logging.Verbosity", value(&logging_verbosity), "Logging Verbosity - Default Info")
		("Repository.path", value(&repPath), "RepositoryDB path")
		("General.Workdir", value(&workdir), "working dir of the cmd")
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
			std::cout << std::string("sbu ( Smart Backup Utility ) : ") + g_version << std::endl;
			std::cout << std::string("Written by ") << std::string(g_developer_name) << std::endl;
			std::cout << desc << std::endl;
			return ExitCode_HelpCalled;
		}

		if (!vm["version"].empty())
		{
			std::cout << std::string("sbu ( Smart Backup Utility ) : ") + g_version << std::endl;
			std::cout << std::string("Written by ") << std::string(g_developer_name) << std::endl;
			return ExitCode_VersionCalled;
		}

		if (vm["action"].empty())
		{
			std::cout << "Missing action argument" << std::endl;
			return ExitCode_MissingAction;
		}
	}
	catch (const boost::program_options::unknown_option exception)
	{
		std::cout << std::string("Invalid option ") << exception.get_option_name() << std::endl;
		retValue = ExitCode_InvalidArgument;
	}

	return retValue;
}
#include "CommandLineAndConfig.h"
#include "boost/filesystem.hpp"
#include <iostream>
#include "ExitCodes.h"
#include "Loggers.h"

using namespace boost::program_options;

#include "sbu.h"

int CommandLineAndOptions::ParseOptions(int argc, const char* argv[])
{
	int retValue = ExitCode_Success;

	const std::string configFileEnvVar = "SBU_CONFIG";
	const long maxSizeToBulkDefault = 128 * 1024;
	const long bulkSizeDefault = 5 * 1024 * 1024;
	const std::string shouldLogToConsole = "False";
	const std::string rootLogLevel = "Information";
	const std::string repositoryDB = "RepositoryDB.db";
	const std::string backupDB = "BackupDB.db";
	const std::string fileRepositoryDB = "FileRepositoryDB.db";

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
	std::string backupDBPath;
	std::string bstep;
	std::string rstep;
	std::string filerepPassword;
	std::string awskey, awssecret, awsbucket, awsregion, awsBasePath;
	std::string storageType;
	int byID;
	long maxSizeToBulk;
	long bulkSize;

	desc.add_options()
		("help,h", "print usage message")
		("version", "print version")
		("config", value(&config_file), "Config file")
		("General.Workdir", value(&workdir), "working dir of the cmd")
		("action,a", value(&action), "Actions include : CreateBackupDef, ListBackupDef, Backup, Restore, ListBackup, BackupInfo")
		("name,n", value(&name), "Name")
		("path,p", value(&path), "Path")
		("byID", value(&byID), "Select By ID")
		("bstep", value(&bstep), "BackupStep : All, Init, Scan, DiffCalc, FileUpload, Complete")
		("rstep", value(&rstep), "RestoreStep: Init, Scan, DiffCalc, Download, Complete")
		("date,d", value(&date), "The last effective date - Defaults to now()")
		("General.StorageType", value(&filerepPassword), "StorageType: FileRepository, SecureFileRepository, AwsS3, SecureAwsS3")
		("Storage.password", value(&storageType), "If exists uses this password SecureFileRepository or S3 Client side encryption")
		("FileRepository.name", value(&filerepName)->default_value(fileRepositoryDB), "The database name of the FileRepository")
		("FileRepository.path", value(&filerepPath), "Path of the FileRepository")
		("FileRepository.maxSizeToBulk", value(&maxSizeToBulk)->default_value(maxSizeToBulkDefault), "Minimum file size to bulk")
		("FileRepository.bulkSize", value(&bulkSize)->default_value(bulkSizeDefault), "bulk Size")
		("AwsS3Storage.key", value(&awskey), "Access key for the AWS Account")
		("AwsS3Storage.secret", value(&awssecret), "Secret for the AWS Account")
		("AwsS3Storage.bucket", value(&awsbucket), "S3 Bucket AWS Account")
		("AwsS3Storage.region", value(&awsregion), "S3 Bucket region AWS Account")
		("AwsS3Storage.path", value(&awsBasePath), "S3 BasePath in Bucket AWS Account")
		("Logging.Console", value(&logging)->default_value(shouldLogToConsole), "true/false - Enable logs to console")
		("Logging.FileOutput", value(&logging), "filename - if exists emit logs to the file")
		("Logging.Verbosity", value(&logging_verbosity)->default_value(rootLogLevel), "Logging Verbosity - Default Info")
		("Repository.path", value(&repPath)->default_value(repositoryDB), "RepositoryDB path")
		("showOnly", "In restore show only the files to be restored")
		("BackupDB.path", value(&backupDBPath)->default_value(backupDB), "The name of the Backup database to use")
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
				auto fileParams = parse_config_file<char>(sbuConfigFileName.c_str(), desc, true);
				for (const auto& o : fileParams.options) {
					const std::string logComponentPrefix = "Logging.Components.";
					if (o.string_key.rfind(logComponentPrefix, 0) == 0)
					{
						LoggingComponentEntry entry;
						entry.componentName = o.string_key.substr(logComponentPrefix.size());
						entry.level = o.value[0];
						loggingOptions.components.push_back(entry);
					}

				}
				store(fileParams, vm);
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

		if (!vm["help"].empty() || !vm["version"].empty())
		{
			std::cout << std::string("sbu ( Smart Backup Utility ) : ") + g_Version << std::endl;
			std::cout << std::string("Written by ") << std::string(g_DeveloperName) << std::endl;

			if (!vm["help"].empty())
			{
				std::cout << desc << std::endl;
				return ExitCode_HelpCalled;
			}
			else
			{
				return ExitCode_VersionCalled;
			}
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

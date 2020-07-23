#include <memory>
#include <stdio.h>
#include <iostream>
#include <map>

#include "CommandLineAndConfig.h"
#include "boost/filesystem.hpp"
#include "boost/stacktrace.hpp"
#include "Loggers.h"
#include "Operations.h"
#include "ExitCodes.h"
#include "utils.h"

extern const std::string g_DeveloperName = "Shai Roitman";
extern const std::string g_Version = "0.9";
extern const std::string g_CopyRight = "Shai Roitman - 2019";

int main(int argc, const char* argv[])
{
	register_stacktrace_handler();

	std::map<std::string, Operation::Factory> operations;
	operations["CreateBackupDef"] = CreateBackupDefFactory;
	operations["ListBackupDef"] = ListBackupDefsFactory;
	operations["Backup"] = BackupFactory;
	operations["Restore"] = RestoreFactory;
	operations["ListBackup"] = ListBackupsFactory;
	operations["BackupInfo"] = BackupInfoFactory;

	CommandLineAndOptions options;
	LoggingOptions loggingComponentsOptions;
	int retValue = options.ParseOptions(argc, argv, loggingComponentsOptions);
	LoggerFactory::InitLogger(options.vm, loggingComponentsOptions);
	static auto logger = LoggerFactory::getLogger("application");

	logger->Info("main(): Application Started");

	if (!options.vm["General.Workdir]"].empty())
	{
		auto workDir = options.vm["General.Workdir]"].as<std::string>();
		boost::filesystem::current_path(workDir);
	}
	logger->InfoFormat("Working Directory [%s]", boost::filesystem::current_path().string().c_str());

	if (retValue == ExitCode_Success)
	{
		std::string action = options.vm["action"].as<std::string>();
		logger->DebugFormat("Application action:[%s]", action.c_str());

		if (operations.find(action) != operations.end())
		{
			try {
				auto factory = operations[action]();
				retValue = factory->Operate(options.vm);
			}
			catch (std::exception ex)
			{
				logger->ErrorFormat("Fail to run action:[%s] got exception:[%s]", action.c_str(), ex.what());
				retValue = ExitCode_GeneralFailure;
			}
		}
		else
		{
			logger->ErrorFormat("Fail to run missing action:[%s]", action.c_str());
			retValue = ExitCode_InvalidAction;
		}
	}

	logger->InfoFormat("main(): Application Ended retValue:[%d]", retValue);
	return retValue;
}

#include <memory>
#include <stdio.h>
#include <iostream>
#include <map>

#include "CommandLineAndConfig.h"
#include "boost/filesystem.hpp"
#include "Loggers.h"
#include "Operations.h"
#include "ExitCodes.h"

static auto logger = LoggerFactory::getLogger("application");

int main(int argc, const char* argv[])
{
	std::map<std::string, std::shared_ptr<Operation>> operations;
	operations["CreateBackupDef"] = std::make_shared<CreateBackupDefOperation>();
	operations["ListBackupDef"] = std::make_shared<ListBackupDefsOperation>();
	operations["Backup"] = std::make_shared<BackupOperation>();
	operations["Restore"] = std::make_shared<RestoreOperation>();
	operations["ListBackup"] = std::make_shared<ListBackupsOperation>();

	CommandLineAndOptions options;
	int retValue = options.ParseOptions(argc, argv);
	LoggerFactory::InitLogger(options.vm);
	logger->Info("Application Started");

	if (!options.vm["General.Workdir]"].empty())
	{
		auto workDir = options.vm["General.Workdir]"].as<std::string>();
		logger->InfoFormat("Working Directory [%s]", workDir.c_str());
		boost::filesystem::current_path(workDir);
	}

	if ( retValue == ExitCode_Success )
	{
		std::string action = options.vm["action"].as<std::string>();
		logger->DebugFormat("Application action:[%s]", action.c_str());

		if (operations.find(action) != operations.end())
		{
			std::shared_ptr<Operation>& operation = operations[action];
			try {
				retValue = operation->Operate(options.vm);
			}
			catch (std::exception ex)
			{
				logger->ErrorFormat("Fail to run action:[%s] got exception:[%s]", action.c_str(), ex.what());
			}
		}
		else
		{
			logger->ErrorFormat("Fail to run missing action:[%s]", action.c_str());
			retValue = ExitCode_InvalidAction;
		}
	}

	logger->InfoFormat("Application Ended retValue:[%d]", retValue);
	return retValue;
}

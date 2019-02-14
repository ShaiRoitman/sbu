#include <memory>
#include <stdio.h>
#include <iostream>
#include <map>

#include "CommandLineAndConfig.h"
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

	if ( retValue == ExitCode_Success )
	{ 
		std::string action = options.vm["action"].as<std::string>();
		if (operations.find(action) != operations.end())
		{
			std::shared_ptr<Operation>& operation = operations[action];
			retValue = operation->Operate(options.vm);
		}
		else
		{
			retValue = ExitCode_InvalidAction;
		}
	}

	return retValue;
}

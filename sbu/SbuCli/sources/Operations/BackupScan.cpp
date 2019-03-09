#include "Operations.h"

#include <iostream>
#include "utils.h"
#include "factories.h"
#include "sbu_exceptions.h"
#include "ExitCodes.h"
#include "BackupDB.h"

static auto logger = LoggerFactory::getLogger("Operations");

class BackupScanOperation : public Operation
{
public:
	int Operate(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;

		std::string dbPath = vm["BackupDB.path"].as<std::string>();

		logger->DebugFormat("Operation:[BackupScanOperation] dbPath:[%s]", dbPath.c_str());

		bool doesExists = boost::filesystem::exists(dbPath);
		auto backupDB = CreateSQLiteDB(dbPath);
		if (doesExists == false)
		{
			std::string name = vm["name"].as<std::string>();
			auto RepoDB = getRepository(vm);
			auto backupdef = RepoDB->GetBackupDef(name);
			if (backupdef != nullptr)
			{
				backupDB->StartScan(backupdef->rootPath);
			}
		}
		else
		{
			backupDB->ContinueScan();
		}

		logger->DebugFormat("Operation:[BackupScanOperation] dbPath:[%s] retValue:[%d]", dbPath.c_str(), retValue);
		return retValue;
	}
};

std::shared_ptr<Operation> BackupScanFactory()
{
	return std::make_shared<BackupScanOperation>();
}

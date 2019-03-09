#include "Operations.h"

#include <iostream>
#include "utils.h"
#include "factories.h"
#include "sbu_exceptions.h"
#include "ExitCodes.h"
#include "BackupDB.h"

static auto logger = LoggerFactory::getLogger("Operations");

class BackupDiffCalcOperation : public Operation
{
public:
	int Operate(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;

		std::string dbPath = vm["BackupDB.path"].as<std::string>();

		logger->DebugFormat("Operation:[BackupDiffCalcOperation] dbPath:[%s]", dbPath.c_str());

		bool doesExists = boost::filesystem::exists(dbPath);
		auto backupDB = CreateSQLiteDB(dbPath);
		if (doesExists)
		{
			std::string name = vm["name"].as<std::string>();
			auto RepoDB = getRepository(vm);
			auto backupdef = RepoDB->GetBackupDef(name);
			RepoDB->CopyCurrentStateIntoBackupDB(dbPath, *backupdef);
			if (backupdef != nullptr)
			{
				backupDB->StartDiffCalc();
				backupDB->ContinueDiffCalc();
			}
		}
		else
		{
			retValue = ExitCode_GeneralFailure;
		}

		logger->DebugFormat("Operation:[BackupDiffCalcOperation] dbPath:[%s] retValue:[%d]", dbPath.c_str(), retValue);
		return retValue;
	}
};

std::shared_ptr<Operation> BackupDiffCalcFactory()
{
	return std::make_shared<BackupDiffCalcOperation>();
}

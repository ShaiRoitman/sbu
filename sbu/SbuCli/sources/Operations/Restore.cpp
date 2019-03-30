#include "Operations.h"

#include <iostream>
#include "utils.h"
#include "factories.h"
#include "sbu_exceptions.h"
#include "ExitCodes.h"

class RestoreOperation : public Operation
{
public:
	RestoreOperation() : logger(LoggerFactory::getLogger("Operations")) {}
	int Operate(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;
		auto restoreParameters = IRepositoryDB::RestoreParameters();

		std::string name = vm["name"].as<std::string>();
		std::string rootDest = vm["path"].as < std::string>();
		restoreParameters.RootDest(rootDest);
		std::string dateStr;
		auto restoreDate = std::chrono::system_clock::now();
		if (!vm["date"].empty())
		{
			dateStr = vm["date"].as<std::string>();
			restoreDate = get_time_point(dateStr);
		}
		restoreParameters.DateToRestore(restoreDate);

		auto showOnly = false;
		if (!vm["showOnly"].empty())
		{
			showOnly = true;
		}
		restoreParameters.ShowOnly(showOnly);

		if (!vm["byID"].empty())
		{
			restoreParameters.byID = vm["byID"].as<int>();
		}

		logger->DebugFormat("Operation:[Restore] Name:[%s] DestPath:[%s] Date:[%s]", name.c_str(), rootDest.c_str(), dateStr.c_str());

		auto RepoDB = getRepository(vm);
		auto backupdef = RepoDB->GetBackupDef(name);
		if (backupdef != nullptr)
		{
			std::shared_ptr<IFileRepositoryDB> fileRepDB = getFileRepository(vm);
			restoreParameters.BackupDefId(backupdef->id);
			auto isRestored = RepoDB->Restore(restoreParameters, fileRepDB);
		}

		logger->InfoFormat("Operation:[Restore] Name:[%s] DestPath:[%s] Date:[%s] retValue:[%d]", name.c_str(), rootDest.c_str(), dateStr.c_str(), retValue);
		return retValue;

	}
private:
	std::shared_ptr<ILogger> logger;
};
std::shared_ptr<Operation> RestoreFactory()
{
	return std::make_shared<RestoreOperation>();

}

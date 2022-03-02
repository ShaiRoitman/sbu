#include "Operations.h"
#include "StandardOutputWrapper.h"

#include <iostream>
#include "utils.h"
#include "factories.h"
#include "sbu_exceptions.h"
#include "ExitCodes.h"
#include "Operations/RestoreOperation.h"

static auto logger = LoggerFactory::getLogger("Operations.Restore");

RestoreOperation::RestoreOperation() 
{
}

RestoreOperation::RestoreOperation(std::shared_ptr<Strategy> strategy)
{
	this->strategy = strategy;
}

int RestoreOperation::Operate(boost::program_options::variables_map& vm)
{
	int retValue = ExitCode_Success;
	auto restoreParameters = IRepositoryDB::RestoreParameters();

	std::string name = getValueAsString(vm, "name");
	std::string rootDest = getValueAsString(vm, "path");
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
	if (showOnly)
	{
		restoreParameters.altToCopyFunc = this->strategy->altToCopy;
	}

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

std::shared_ptr<Operation> RestoreFactory()
{
	auto retValue = std::make_shared<RestoreOperation>();
	retValue->strategy = std::make_shared<RestoreOperation::Strategy>();
	retValue->strategy->altToCopy = [](boost::filesystem::path& destination)
	{
		std::cout << "Restore -> [" << destination << "]" << std::endl;
	};

	return retValue;
}

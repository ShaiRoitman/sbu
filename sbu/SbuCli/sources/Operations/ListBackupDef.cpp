#include "Operations.h"

#include <iostream>
#include "utils.h"
#include "factories.h"
#include "sbu_exceptions.h"
#include "ExitCodes.h"

static auto logger = LoggerFactory::getLogger("Operations.ListBackupDef");

class ListBackupDefsOperation : public Operation
{
public:
	class Strategy
	{
	public:
		std::function<void(const IRepositoryDB::BackupDef& backupdef)> backupDefIter;
	};
	
	ListBackupDefsOperation() {}

	int Operate(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;

		logger->DebugFormat("Operation:[ListBackupDefs]");
		auto RepoDB = getRepository(vm);
		auto backupdefs = RepoDB->GetBackupDefs();
		for (auto iter = backupdefs.begin(); iter != backupdefs.end(); ++iter)
		{
			auto backupdef = *iter;
			if (this->strategy != nullptr && this->strategy->backupDefIter != nullptr)
			{
				this->strategy->backupDefIter(backupdef);
			}
		}

		logger->DebugFormat("Operation:[ListBackupDefs] retValue:[%d]", retValue);
		return retValue;
	}
public:
	std::shared_ptr<Strategy> strategy;
};

std::shared_ptr<Operation> ListBackupDefsFactory()
{
	auto retValue = std::make_shared<ListBackupDefsOperation>();
	retValue->strategy = std::make_shared<ListBackupDefsOperation::Strategy>();
	retValue->strategy->backupDefIter =
		[](const IRepositoryDB::BackupDef& backupdef)
	{
		std::cout << backupdef.id << ",";
		std::cout << backupdef.name << ",";
		std::cout << backupdef.hostName << ",";
		std::cout << backupdef.rootPath << ",";
		std::cout << get_string_from_time_point(backupdef.added) << std::endl;
	};
	return retValue;
}

#include "Operations.h"
#include "StandardOutputWrapper.h"

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
		if (this->strategy != nullptr && this->strategy->backupDefIter != nullptr)
		{
			RepoDB->ListBackupDefs(this->strategy->backupDefIter);
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
		StandardOutputWrapper& output = *StandardOutputWrapper::GetInstance();
		output << backupdef.id << ",";
		output << backupdef.name << ",";
		output << backupdef.hostName << ",";
		output << backupdef.rootPath << ",";
		output << get_string_from_time_point(backupdef.added);
		output.EOL();
	};
	return retValue;
}

#include "Operations.h"
#include "StandardOutputWrapper.h"

#include <iostream>
#include "utils.h"
#include "factories.h"
#include "sbu_exceptions.h"
#include "ExitCodes.h"

static auto logger = LoggerFactory::getLogger("Operations.ListBackup");

class ListBackupsOperation : public Operation
{
public:
	class Strategy
	{
	public:
		std::function<void(const IRepositoryDB::BackupInfo& backup)> backupIter;
	};

	ListBackupsOperation() {}
	int Operate(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;

		std::string name = getValueAsString(vm, "name");

		logger->DebugFormat("Operation:[ListBackups] Name:[%s]", name.c_str());

		auto RepoDB = getRepository(vm);
		auto backupdef = RepoDB->GetBackupDef(name);
		if (backupdef != nullptr)
		{
			RepoDB->ListBackups(backupdef->id, this->strategy->backupIter);
		}
		logger->DebugFormat("Operation:[ListBackups] Name:[%s] retValue:[%d]", name.c_str(), retValue);

		return retValue;
	}
public:
	std::shared_ptr<Strategy> strategy;
};

std::shared_ptr<Operation> ListBackupsFactory()
{
	auto retValue = std::make_shared<ListBackupsOperation>();
	retValue->strategy = std::make_shared<ListBackupsOperation::Strategy>();
	retValue->strategy->backupIter = [](const IRepositoryDB::BackupInfo& backup)
	{
		StandardOutputWrapper& output = *StandardOutputWrapper::GetInstance();
		output << backup.id << ",";
		output << backup.status << ",";
		output << get_string_from_time_point(backup.started) << ",";
		output << get_string_from_time_point(backup.lastUpdated);
		output.EOL();
	};

	return retValue;
}

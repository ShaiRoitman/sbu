#include "Operations.h"

#include <iostream>
#include "utils.h"
#include "factories.h"
#include "sbu_exceptions.h"
#include "ExitCodes.h"

static auto logger = LoggerFactory::getLogger("Operations");

class ListBackupDefsOperation : public Operation
{
public:
	void Print(IRepositoryDB::BackupDef& backupdef)
	{
		std::cout << backupdef.id << ",";
		std::cout << backupdef.name << ",";
		std::cout << backupdef.hostName << ",";
		std::cout << backupdef.rootPath << ",";
		std::cout << get_string_from_time_point(backupdef.added) << std::endl;
	}
	int Operate(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;

		logger->DebugFormat("Operation:[ListBackupDefs]");
		auto RepoDB = getRepository(vm);
		auto backupdefs = RepoDB->GetBackupDefs();
		for (auto iter = backupdefs.begin(); iter != backupdefs.end(); ++iter)
		{
			auto backupdef = *iter;
			Print(backupdef);
		}

		logger->DebugFormat("Operation:[ListBackupDefs] retValue:[%d]", retValue);
		return retValue;
	}
};
std::shared_ptr<Operation> ListBackupDefsFactory()
{
	return std::make_shared<ListBackupDefsOperation>();
}

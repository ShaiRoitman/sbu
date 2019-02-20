#include "Operations.h"

#include <iostream>
#include "utils.h"
#include "factories.h"
#include "sbu_exceptions.h"
#include "ExitCodes.h"

static auto logger = LoggerFactory::getLogger("Operations");

class BackupOperation : public Operation
{
public:
	int Operate(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;

		std::string name = vm["name"].as<std::string>();

		logger->DebugFormat("Operation:[Backup] Name:[%s]", name.c_str());

		auto RepoDB = getRepository(vm);
		auto backupdef = RepoDB->GetBackupDef(name);
		if (backupdef != nullptr)
		{
			std::shared_ptr<IFileRepositoryDB> fileRepDB = getFileRepository(vm);
			auto backupInfo = RepoDB->Backup(IRepositoryDB::BackupParameters().BackupDefId(backupdef->id), fileRepDB);
		}

		logger->DebugFormat("Operation:[Backup] Name:[%s] retValue:[%d]", name.c_str(), retValue);
		return retValue;
	}
};
std::shared_ptr<Operation> BackupFactory()
{
	return std::make_shared<BackupOperation>();
}

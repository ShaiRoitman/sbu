#include "Operations.h"

#include <iostream>
#include "utils.h"
#include "factories.h"
#include "sbu_exceptions.h"
#include "ExitCodes.h"

static auto logger = LoggerFactory::getLogger("Operations");

class ListBackupsOperation : public Operation
{
public:
	void Print(IRepositoryDB::BackupInfo& backup)
	{
		std::cout << backup.id << ",";
		std::cout << backup.status << ",";
		std::cout << get_string_from_time_point(backup.started) << ",";
		std::cout << get_string_from_time_point(backup.lastUpdated) << std::endl;
	}
	int Operate(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;

		std::string name = vm["name"].as<std::string>();

		logger->DebugFormat("Operation:[ListBackups] Name:[%s]", name.c_str());

		auto RepoDB = getRepository(vm);
		auto backupdef = RepoDB->GetBackupDef(name);
		if (backupdef != nullptr)
		{

			auto backups = RepoDB->GetBackups(backupdef->id);
			for (auto iter = backups.begin(); iter != backups.end(); ++iter)
			{
				auto backup = *iter;
				Print(backup);
			}
		}

		logger->DebugFormat("Operation:[ListBackups] Name:[%s] retValue:[%d]", name.c_str(), retValue);

		return retValue;
	}
};
std::shared_ptr<Operation> ListBackupsFactory()
{
	return std::make_shared<ListBackupsOperation>();
}

#include "Operations.h"

#include <iostream>
#include "utils.h"
#include "factories.h"
#include "sbu_exceptions.h"
#include "ExitCodes.h"

class ListBackupsOperation : public Operation
{
public:
	ListBackupsOperation() : logger(LoggerFactory::getLogger("Operations")) {}
	int Operate(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;

		std::string name = vm["name"].as<std::string>();

		logger->DebugFormat("Operation:[ListBackups] Name:[%s]", name.c_str());

		auto RepoDB = getRepository(vm);
		auto backupdef = RepoDB->GetBackupDef(name);
		if (backupdef != nullptr)
		{
			RepoDB->ListBackups(backupdef->id);
		}

		logger->DebugFormat("Operation:[ListBackups] Name:[%s] retValue:[%d]", name.c_str(), retValue);

		return retValue;
	}
private:
	std::shared_ptr<ILogger> logger;

};
std::shared_ptr<Operation> ListBackupsFactory()
{
	return std::make_shared<ListBackupsOperation>();
}

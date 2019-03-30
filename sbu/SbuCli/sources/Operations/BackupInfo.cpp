
#include "Operations.h"

#include <iostream>
#include "utils.h"
#include "factories.h"
#include "sbu_exceptions.h"
#include "ExitCodes.h"

class BackupInfoOperation : public Operation
{
public:
	BackupInfoOperation() : logger(LoggerFactory::getLogger("Operations")) {}
	int Operate(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;

		Integer backupid = vm["byID"].as<int>();

		logger->DebugFormat("Operation:[BackupInfo] Id:[%d]", backupid);

		auto RepoDB = getRepository(vm);
		RepoDB->ListBackupInfo(backupid);

		logger->DebugFormat("Operation:[BackupInfo] Id:[%d] retValue:[%d]", backupid, retValue);

		return 0;
	}
private:
	std::shared_ptr<ILogger> logger;
}; 
std::shared_ptr<Operation> BackupInfoFactory()
{
	return std::make_shared<BackupInfoOperation>();
}
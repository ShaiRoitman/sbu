
#include "Operations.h"

#include <iostream>
#include "utils.h"
#include "factories.h"
#include "sbu_exceptions.h"
#include "ExitCodes.h"

static auto logger = LoggerFactory::getLogger("Operations");

class BackupInfoOperation : public Operation
{
public:
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
}; 
std::shared_ptr<Operation> BackupInfoFactory()
{
	return std::make_shared<BackupInfoOperation>();
}
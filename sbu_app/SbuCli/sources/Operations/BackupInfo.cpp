
#include "Operations.h"
#include "StandardOutputWrapper.h"

#include <iostream>
#include "utils.h"
#include "factories.h"
#include "sbu_exceptions.h"
#include "ExitCodes.h"

static auto logger = LoggerFactory::getLogger("Operations.BackupInfo");

class BackupInfoOperation : public Operation
{
public:
	class Strategy
	{
	public:
		std::function<void(const std::string& status, const std::string& path, const std::string& type)> fileInfoFunc;
	};

	BackupInfoOperation() {}
	int Operate(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;

		Integer backupid = getValueAsInt(vm, "byID");

		logger->DebugFormat("Operation:[BackupInfo] Id:[%d]", backupid);

		auto RepoDB = getRepository(vm);
		RepoDB->ListBackupInfo(backupid, this->strategy->fileInfoFunc);
		logger->DebugFormat("Operation:[BackupInfo] Id:[%d] retValue:[%d]", backupid, retValue);

		return 0;
	}
public:
	std::shared_ptr<Strategy> strategy;
};

std::shared_ptr<Operation> BackupInfoFactory()
{
	auto retValue = std::make_shared<BackupInfoOperation>();
	retValue->strategy = std::make_shared<BackupInfoOperation::Strategy>();
	retValue->strategy->fileInfoFunc =
		[](const std::string& status, const std::string& path, const std::string& type)
	{
		StandardOutputWrapper::GetInstance()->OutputLine(status + " : " + path + " : " + type);
	};
	return retValue;
}
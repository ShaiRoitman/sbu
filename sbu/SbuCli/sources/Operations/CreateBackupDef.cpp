#include "Operations.h"

#include <iostream>
#include "utils.h"
#include "factories.h"
#include "sbu_exceptions.h"
#include "ExitCodes.h"

static auto logger = LoggerFactory::getLogger("Operations.CreateBackupDef");

class CreateBackupDefOperation : public Operation
{
public:
	CreateBackupDefOperation() {}

	void Print(std::shared_ptr<IRepositoryDB::BackupDef> backupdef)
	{
		std::cout << backupdef->id << ",";
		std::cout << backupdef->name << ",";
		std::cout << backupdef->hostName << ",";
		std::cout << backupdef->rootPath << ",";
		std::cout << get_string_from_time_point(backupdef->added) << std::endl;
	}

	virtual int Operate(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;

		std::string name = vm["name"].as<std::string>();
		std::string path = vm["path"].as<std::string>();

		logger->DebugFormat("Operation:[CreateBackupDef] Name:[%s] Path:[%s]", name.c_str(), path.c_str());

		auto RepoDB = getRepository(vm);
		try {
			auto backupdef = RepoDB->AddBackupDef(name, boost::filesystem::path(path));
			Print(backupdef);
		}
		catch (sbu_alreadyexists ex)
		{
			logger->ErrorFormat("CreateBackupDefOperation::Operate() Failed due to duplicate [%s]", name.c_str());
			retValue = ExitCode_AlreadyExists;
		}

		logger->InfoFormat("Operation :[CreateBackupDef] Name:[%s] Path:[%s] retValue:[%d]", name.c_str(), path.c_str(), retValue);
		return retValue;
	}
};

std::shared_ptr<Operation> CreateBackupDefFactory()
{
	return std::make_shared<CreateBackupDefOperation>();
}

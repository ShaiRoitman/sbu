#include "Operations.h"

#include <iostream>
#include "utils.h"
#include "factories.h"
#include "sbu_exceptions.h"
#include "ExitCodes.h"

static auto logger = LoggerFactory::getLogger("Operations");

class CreateBackupDefOperation : public Operation
{
public:
	void Print(std::shared_ptr<IRepositoryDB::BackupDef> backupdef)
	{
		std::cout << backupdef->id << ",";
		std::cout << backupdef->name << ",";
		std::cout << backupdef->hostName << ",";
		std::cout << backupdef->rootPath << ",";
		std::cout << get_string_from_time_point(backupdef->added) << "\n";
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

class ListBackupDefsOperation : public Operation
{
public:
	void Print(IRepositoryDB::BackupDef& backupdef)
	{
		std::cout << backupdef.id << ",";
		std::cout << backupdef.name << ",";
		std::cout << backupdef.hostName << ",";
		std::cout << backupdef.rootPath << ",";
		std::cout << get_string_from_time_point(backupdef.added) << "\n";
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

class RestoreOperation : public Operation
{
public:
	int Operate(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;

		std::string name = vm["name"].as<std::string>();
		std::string rootDest = vm["path"].as < std::string>();
		std::string dateStr;
		auto restoreDate = std::chrono::system_clock::now();
		if (!vm["date"].empty())
		{
			dateStr = vm["date"].as<std::string>();
			restoreDate = get_time_point(dateStr);
		}

		auto showOnly = false;
		if (!vm["showOnly"].empty())
		{
			showOnly = true;
		}

		logger->DebugFormat("Operation:[Restore] Name:[%s] DestPath:[%s] Date:[%s]", name.c_str(), rootDest.c_str(), dateStr.c_str());

		auto RepoDB = getRepository(vm);
		auto backupdef = RepoDB->GetBackupDef(name);
		if (backupdef != nullptr)
		{
			std::shared_ptr<IFileRepositoryDB> fileRepDB = getFileRepository(vm);
			auto isRestored = RepoDB->Restore(
				IRepositoryDB::RestoreParameters().
				BackupDefId(backupdef->id).RootDest(rootDest).ShowOnly(showOnly).DateToRestore(restoreDate),
				fileRepDB);
		}

		logger->InfoFormat("Operation:[Restore] Name:[%s] DestPath:[%s] Date:[%s] retValue:[%d]", name.c_str(), rootDest.c_str(), dateStr.c_str(), retValue);
		return retValue;

	}
};
std::shared_ptr<Operation> RestoreFactory()
{
	return std::make_shared<RestoreOperation>();

}

class ListBackupsOperation : public Operation
{
public:
	void Print(IRepositoryDB::BackupInfo& backup)
	{
		std::cout << backup.id << ",";
		std::cout << backup.status << ",";
		std::cout << get_string_from_time_point(backup.started) << ",";
		std::cout << get_string_from_time_point(backup.lastUpdated) << ",";
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


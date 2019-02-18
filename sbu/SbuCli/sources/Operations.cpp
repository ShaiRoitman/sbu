#include "Operations.h"

#include <iostream>
#include "utils.h"
#include "factories.h"
#include "sbu_exceptions.h"
#include "ExitCodes.h"

static auto logger = LoggerFactory::getLogger("Operations");

int CreateBackupDefOperation::Operate(boost::program_options::variables_map& vm)
{
	int retValue = ExitCode_Success;

	std::string name = vm["name"].as<std::string>();
	std::string path = vm["path"].as<std::string>();

	logger->DebugFormat("Operation:[CreateBackupDef] Name:[%s] Path:[%s]", name.c_str(), path.c_str());

	auto RepoDB = getRepository(vm);
	try {
		auto backupdef = RepoDB->AddBackupDef(name, boost::filesystem::path(path));
		std::cout << backupdef->id << ",";
		std::cout << backupdef->name << ",";
		std::cout << backupdef->hostName << ",";
		std::cout << backupdef->rootPath << ",";
		std::cout << get_string_from_time_point(backupdef->added) << "\n";
	}
	catch(sbu_alreadyexists ex)
	{
		logger->ErrorFormat("CreateBackupDefOperation::Operate() Failed due to duplicate [%s]", name.c_str());
		retValue = ExitCode_AlreadyExists;
	}

	logger->InfoFormat("Operation :[CreateBackupDef] Name:[%s] Path:[%s] retValue:[%d]", name.c_str(), path.c_str(), retValue);
	return retValue;
}

int ListBackupDefsOperation::Operate(boost::program_options::variables_map& vm)
{
	int retValue = ExitCode_Success;

	logger->DebugFormat("Operation:[ListBackupDefs]");
	auto RepoDB = getRepository(vm);
	auto backupdefs = RepoDB->GetBackupDefs();
	std::list<IRepositoryDB::BackupDef>::iterator iter;
	for (iter = backupdefs.begin(); iter != backupdefs.end(); ++iter)
	{
		auto backupdef = *iter;
		std::cout << backupdef.id << ",";
		std::cout << backupdef.name << ",";
		std::cout << backupdef.hostName << ",";
		std::cout << backupdef.rootPath << ",";
		std::cout << get_string_from_time_point(backupdef.added) << "\n";
	}

	logger->DebugFormat("Operation:[ListBackupDefs] retValue:[%d]", retValue);
	return retValue;
}

int RestoreOperation::Operate(boost::program_options::variables_map& vm)
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

	logger->InfoFormat("Operation:[Restore] Name:[%s] DestPath:[%s] Date:[%s] retValue:[%s]", name.c_str(), rootDest.c_str(), dateStr.c_str(), retValue);
	return retValue;
}

int BackupOperation::Operate(boost::program_options::variables_map& vm)
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

int ListBackupsOperation::Operate(boost::program_options::variables_map& vm)
{
	int retValue = ExitCode_Success;

	std::string name = vm["name"].as<std::string>();

	logger->DebugFormat("Operation:[ListBackups] Name:[%s]", name.c_str());

	auto RepoDB = getRepository(vm);
	auto backupdef = RepoDB->GetBackupDef(name);
	if (backupdef != nullptr)
	{

		auto backups = RepoDB->GetBackups(backupdef->id);
		std::list<IRepositoryDB::BackupInfo>::iterator iter;
		for (iter = backups.begin(); iter != backups.end(); ++iter)
		{
			auto backup = *iter;
			std::cout << backup.id << ",";
			std::cout << backup.status << ",";
			std::cout << get_string_from_time_point(backup.started) << ",";
			std::cout << get_string_from_time_point(backup.ended) << ",";
		}
	}

	logger->DebugFormat("Operation:[ListBackups] Name:[%s] retValue:[%d]", name.c_str(), retValue);
	return retValue;
}

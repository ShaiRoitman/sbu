#include "Operations.h"

#include <iostream>
#include "utils.h"
#include "RepositoryDB.h"


int CreateBackupDefOperation::Operate(boost::program_options::variables_map& vm)
{
	std::string repoDB = "repoDB.db";
	std::shared_ptr<IRepositoryDB> RepoDB = CreateRepositorySQLiteDB(repoDB);

	std::string name = vm["name"].as<std::string>();
	std::string path = vm["path"].as<std::string>();
	auto backupdef = RepoDB->AddBackupDef(name, boost::filesystem::path(path));
	std::cout << backupdef->id << ",";
	std::cout << backupdef->name << ",";
	std::cout << backupdef->hostName << ",";
	std::cout << backupdef->rootPath << ",";
	std::cout << get_string_from_time_point(backupdef->added) << "\n";

	return 0;
}

int ListBackupDefsOperation::Operate(boost::program_options::variables_map& vm)
{
	std::string repoDB = "repoDB.db";
	std::shared_ptr<IRepositoryDB> RepoDB = CreateRepositorySQLiteDB(repoDB);
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

	return 0;
}

int RestoreOperation::Operate(boost::program_options::variables_map& vm)
{
	std::string repoDB = "repoDB.db";
	std::shared_ptr<IRepositoryDB> RepoDB = CreateRepositorySQLiteDB(repoDB);
	std::string name = vm["name"].as<std::string>();
	auto backupdef = RepoDB->GetBackupDef(name);
	auto rootDest = vm["path"].as < std::string>();
	auto restoreDate = std::chrono::system_clock::now();
	if (!vm["date"].empty())
	{
		restoreDate = get_time_point(vm["date"].as<std::string>());
	}
	auto showOnly = false;
	if (!vm["showOnly"].empty())
	{
		showOnly = true;
	}
	if (backupdef != nullptr)
	{
		std::shared_ptr<IFileRepositoryDB> fileRepDB = getFileRepository(vm);
		auto isRestored = RepoDB->Restore(
			IRepositoryDB::RestoreParameters().
			BackupDefId(backupdef->id).RootDest(rootDest).ShowOnly(showOnly).DateToRestore(restoreDate),
			fileRepDB);
	}
	return 0;
}

int BackupOperation::Operate(boost::program_options::variables_map& vm)
{
	std::string repoDB = "repoDB.db";
	std::shared_ptr<IRepositoryDB> RepoDB = CreateRepositorySQLiteDB(repoDB);
	std::string name = vm["name"].as<std::string>();
	auto backupdef = RepoDB->GetBackupDef(name);
	if (backupdef != nullptr)
	{
		std::shared_ptr<IFileRepositoryDB> fileRepDB = getFileRepository(vm);
		auto backupInfo = RepoDB->Backup(IRepositoryDB::BackupParameters().BackupDefId(backupdef->id), fileRepDB);
	}

	return 0;
}

int ListBackupsOperation::Operate(boost::program_options::variables_map& vm)
{
	std::string repoDB = "repoDB.db";
	std::shared_ptr<IRepositoryDB> RepoDB = CreateRepositorySQLiteDB(repoDB);
	std::string name = vm["name"].as<std::string>();
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
	return 0;
}

#include <stdio.h>
#include <iostream>
#include "BackupDB.h"
#include "FileRepositoryDB.h"
#include "RepositoryDB.h"
#include "CommandLineAndConfig.h"

using namespace boost::program_options;

int main(int argc, const char* argv[])
{
	CommandLineAndOptions options;
	int retValue = options.ParseOptions(argc, argv);
	if ( retValue != 0)
	{ 
		return retValue;
	}

	std::string action = options.vm["action"].as<std::string>();

	std::string repoDB = "repoDB.db";
	remove(repoDB.c_str());
	std::shared_ptr<IRepositoryDB> RepoDB = CreateRepositorySQLiteDB(repoDB);
	if (action == "CreateBackupDef")
	{
		std::string name = options.vm["name"].as<std::string>();
		std::string path = options.vm["path"].as<std::string>();
		auto backupdef = RepoDB->AddBackupDef(name, boost::filesystem::path(path));
		std::cout << backupdef->id << ",";
		std::cout << backupdef->name << ",";
		std::cout << backupdef->hostName << ",";
		std::cout << backupdef->rootPath << ",";
		std::cout << get_string_from_time_point(backupdef->added) << "\n";
	}
	else if (action == "ListBackupDef")
	{
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
	}
	else if (action == "Backup")
	{
		std::string name = options.vm["name"].as<std::string>();
		auto backupdef = RepoDB->GetBackupDef(name);
		if (backupdef != nullptr)
		{
			auto backupId = RepoDB->Backup(IRepositoryDB::BackupParameters().BackupDefId(backupdef->id));
		}
	}
	else if (action == "Restore")
	{
		std::string name = options.vm["name"].as<std::string>();
		auto backupdef = RepoDB->GetBackupDef(name);
		auto rootDest = options.vm["path"].as < std::string>();
		if (backupdef != nullptr)
		{
			auto restorea = RepoDB->Restore(IRepositoryDB::RestoreParameters().BackupDefId(backupdef->id).RootDest(rootDest));
		}
	}
	else if (action == "ListBackup")
	{
		std::string name = options.vm["name"].as<std::string>();
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
	}

#ifdef TEST 
	auto backupDef = RepoDB->AddBackupDef("Shai", "c:\\git\\clu");
	auto values = RepoDB->GetBackupDefs();
	auto firstBackupDef = *values.begin(); 
	auto tp = get_string_from_time_point(firstBackupDef.added);

	auto backupId = RepoDB->Backup(IRepositoryDB::BackupParameters().BackupDefId(firstBackupDef.id));

	boost::filesystem::remove_all("c:\\git\\clu2");
	auto restorea = RepoDB->Restore(IRepositoryDB::RestoreParameters().BackupDefId(firstBackupDef.id).RootDest("c:\\git\\clu2"));

	//auto backupDeleted = RepoDB->DeleteBackup(backupId.id);
	//bool deleted = RepoDB->DeleteBackupDef(backupDef.id);

	std::string databaseName = "backupDb.db";
	printf("Smart Backup Utility\n");
	remove(databaseName.c_str());
	std::shared_ptr<IBackupDB> backupDB = CreateSQLiteDB(databaseName);
	backupDB->StartScan("C:\\git\\sbu\\sbu\\SbuCli");
	
	std::string fileRepoDB = "fileRepo.db";
	remove(fileRepoDB.c_str());
	std::shared_ptr<IFileRepositoryDB> fileRepDB = CreateFileRepositorySQLiteDB(fileRepoDB, "C:\\git\\sbu\\sbu\\SbuCli\\Repo");
	std::string fileHandle = fileRepDB->AddFile("C:\\git\\sbu\\sbu\\SbuCli\\sources\\sbu.cpp");
	fileRepDB->GetFile(fileHandle, "C:\\git\\sbu\\sbu\\SbuCli\\sources\\sbu.cpp2");
	
	std::string repoDB = "repoDB.db";
	remove(repoDB.c_str());
	std::shared_ptr<IRepositoryDB> RepoDB = CreateRepositorySQLiteDB(repoDB);
	RepoDB->SetFileRepositoryDB(fileRepDB);
#endif
	return 0;
}


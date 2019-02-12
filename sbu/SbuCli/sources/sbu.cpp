#include <stdio.h>
#include <iostream>

#include "BackupDB.h"
#include "FileRepositoryDB.h"
#include "RepositoryDB.h"

#include "CommandLineAndConfig.h"
#include "Loggers.h"

using namespace boost::program_options;

std::shared_ptr<IFileRepositoryDB> getFileRepository(variables_map& vm)
{
	boost::filesystem::path repoPath = vm["FileRepository.path"].as<std::string>();
	boost::filesystem::path dbPath = vm["FileRepository.name"].as<std::string>();
	std::shared_ptr<IFileRepositoryDB> fileRepDB = CreateFileRepositorySQLiteDB(dbPath, repoPath);
	return fileRepDB;
}

class Test
{
public:
	Test(const char* s)
	{
		printf("ctor %s\n",s);
	}

	Test& operator()(const char*s) { printf("%s", s); return *this; }
	~Test()
	{
		printf("dtor\n");
	}
};

int main(int argc, const char* argv[])
{
	CommandLineAndOptions options;
	int retValue = options.ParseOptions(argc, argv);
	LoggerFactory::InitLogger(options.vm);
	auto logger = LoggerFactory::getLogger("application");

	if ( retValue != 0)
	{ 
		return retValue;
	}

	std::string action = options.vm["action"].as<std::string>();

	std::string repoDB = "repoDB.db";
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
			std::shared_ptr<IFileRepositoryDB> fileRepDB = getFileRepository(options.vm);
			auto backupInfo = RepoDB->Backup(IRepositoryDB::BackupParameters().BackupDefId(backupdef->id), fileRepDB);
		}
	}
	else if (action == "Restore")
	{
		std::string name = options.vm["name"].as<std::string>();
		auto backupdef = RepoDB->GetBackupDef(name);
		auto rootDest = options.vm["path"].as < std::string>();
		if (backupdef != nullptr)
		{
			std::shared_ptr<IFileRepositoryDB> fileRepDB = getFileRepository(options.vm);
			auto isRestored = RepoDB->Restore(IRepositoryDB::RestoreParameters().BackupDefId(backupdef->id).RootDest(rootDest), fileRepDB);
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

	return 0;
}


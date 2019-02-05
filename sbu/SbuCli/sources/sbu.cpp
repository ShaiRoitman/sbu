#include <stdio.h>
#include "BackupDB.h"
#include "FileRepositoryDB.h"
#include "RepositoryDB.h"
#include "boost/program_options.hpp"

using namespace boost::program_options;

int main(int argc, const char* argv[])
{
	options_description desc("Allowed options");
	std::string path;
	std::string name;
	bool createdef;
	bool listdef;
	bool backup;
	bool restore;
	bool listbackups;

	desc.add_options()
		("help,h", "print usage message")
		("createdef", bool_switch(&createdef), "create backup def")
		("listdef", bool_switch(&listdef), "list backup def")
		("path,p", value(&path), "Path")
		("name,n", value(&name), "Name")
		;

	variables_map vm;
	store(parse_command_line(argc, argv, desc), vm);
	store(parse_config_file<char>("sbu.config", desc, false), vm);
	store(parse_environment(desc,
		[](const std::string& i_env_var)
	{// maps environment variable "HOSTNAME" to user-defined option "hostname"
		return i_env_var == "HOSTNAME" ? "hostname" : "";
	}),
		vm);

	std::string repoDB = "repoDB.db";
	remove(repoDB.c_str());
	std::shared_ptr<IRepositoryDB> RepoDB = CreateRepositorySQLiteDB(repoDB);

	auto backupDef = RepoDB->AddBackupDef("Shai", "c:\\git\\clu");
	auto values = RepoDB->GetBackupDefs();
	auto firstBackupDef = *values.begin(); 
	auto tp = get_string_from_time_point(firstBackupDef.added);

	auto backupId = RepoDB->Backup(IRepositoryDB::BackupParameters().BackupDefId(firstBackupDef.id));

	boost::filesystem::remove_all("c:\\git\\clu2");
	auto restorea = RepoDB->Restore(IRepositoryDB::RestoreParameters().BackupDefId(firstBackupDef.id).RootDest("c:\\git\\clu2"));

	//auto backupDeleted = RepoDB->DeleteBackup(backupId.id);
	//bool deleted = RepoDB->DeleteBackupDef(backupDef.id);

#ifdef TEST 
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


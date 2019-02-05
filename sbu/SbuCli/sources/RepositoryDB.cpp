#include "RepositoryDB.h"
#include "BackupDB.h"
#include "FileRepositoryDB.h"

#include "SQLiteCpp/SQLiteCpp.h"
#include "utils.h"

using namespace boost::filesystem;
using namespace SQLite;

class RepositoryDB : public IRepositoryDB
{
public:
	RepositoryDB(boost::filesystem::path dbPath)
	{
		bool dbexists = exists(this->dbPath);
		this->dbPath = dbPath;
		db = std::make_shared<SQLite::Database>(dbPath.string(), SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);
		if (!dbexists)
		{
			static std::string createQuery = Text_Resource::Repository;
			db->exec(createQuery);
		}
	}

	virtual void SetFileRepositoryDB(std::shared_ptr<IFileRepositoryDB> fileDB) override
	{
		this->fileDB = fileDB;
	}

	virtual BackupDef AddBackupDef(const std::string& name, boost::filesystem::path rootPath) override
	{
		BackupDef retValue;
		SQLite::Statement insertQuery(*db, "INSERT INTO BackupDefs (Name, Hostname, RootPath, Added) Values (:name, :host, :root, :added)");

		insertQuery.bind(":name", name);
		insertQuery.bind(":host", getHostName());
		insertQuery.bind(":root", to_utf8(rootPath));
		insertQuery.bind(":added", return_current_time_and_date());
		insertQuery.exec();

		SQLite::Statement query(*db, "SELECT ID FROM BackupDefs WHERE Name=:name");
		query.bind(":name", name);
		if (query.executeStep())
		{
			retValue.id = query.getColumn("ID").getInt();
		}

		return retValue;
	}

	virtual bool DeleteBackupDef(Integer id) override
	{
		Transaction transaction(*db);

		SQLite::Statement deleteBackups(*db, "DELETE FROM Backups WHERE BackupDefID = :id");
		deleteBackups.bind(":id", id);
		deleteBackups.exec();

		SQLite::Statement deleteBackupDefs(*db, "DELETE FROM BackupDefs WHERE ID= :id");
		deleteBackupDefs.bind(":id", id);
		bool retValue = deleteBackupDefs.exec() > 0;

		transaction.commit();

		return retValue;
	}

	virtual std::list<BackupDef> GetBackupDefs() override
	{
		std::list<BackupDef> retValue;
		SQLite::Statement selectBackupDefs(*db, "SELECT ID, Name, Hostname, RootPath, Added FROM BackupDefs");

		while (selectBackupDefs.executeStep())
		{
			BackupDef newValue;
			newValue.id = selectBackupDefs.getColumn("ID").getInt();
			newValue.name = selectBackupDefs.getColumn("Name").getString();
			newValue.hostName = selectBackupDefs.getColumn("Hostname").getString();
			newValue.rootPath = from_utf8(selectBackupDefs.getColumn("RootPath").getString());
			newValue.added = get_time_point(selectBackupDefs.getColumn("Added").getString());
			
			retValue.push_back(newValue);
		}
		return retValue;
	}

	virtual BackupInfo Backup(BackupParameters backupParams) override
	{
		BackupInfo retValue;
		retValue.backupDefId = backupParams.backupDefId;
		retValue.started = std::chrono::system_clock::now();
		retValue.status = "Started";
		SQLite::Statement insertBackup(*db, "INSERT INTO Backups (BackupDefID, Started, Status ) VALUES ( :backupdefID, :started, :status )");
		insertBackup.bind(":backupdefID", retValue.backupDefId);
		insertBackup.bind(":started", return_current_time_and_date());
		insertBackup.bind(":status", "Started");
		if (insertBackup.exec() > 0)
		{
			SQLite::Statement selectBackup(*db, "SELECT ID FROM Backups WHERE BackupDefID=:backupdefID ORDER BY ID DESC LIMIT 1");
			selectBackup.bind(":backupdefID", retValue.backupDefId);
			if (selectBackup.executeStep())
			{
				retValue.id = selectBackup.getColumn("ID").getInt();
			}
		}

		auto defs = GetBackupDefs();
		BackupDef def;
		for (std::list<BackupDef>::iterator iter = defs.begin(); iter != defs.end(); ++iter)
		{
			if ((*iter).id == retValue.backupDefId)
			{
				def = *iter;
				break;
			}
		}

		boost::filesystem::remove("backupDB.db");

		std::shared_ptr<IBackupDB> backupDB = CreateSQLiteDB("backupDB.db");
		backupDB->StartScan(def.rootPath);

		SQLite::Statement attach(*db, "ATTACH DATABASE 'backupDB.db' AS BackupDB");
		attach.exec();

		try {
			SQLite::Statement currentStateQuery(*db, Text_Resource::CurrentState);
			currentStateQuery.bind(":backupDefID", def.id);
			currentStateQuery.exec();
		}
		catch (std::exception ex)
		{
			int k = 3;
		}

		SQLite::Statement detach(*db, "DETACH DATABASE BackupDB");
		detach.exec();

		backupDB->StartDiffCalc();
		backupDB->ContinueDiffCalc();

		boost::filesystem::path repoPath("C:\\git\\sbu\\sbu\\SbuCli\\Repo");
		boost::filesystem::remove("fileRepo.db");
		try {
		//	boost::filesystem::remove_all(repoPath);
		}
		catch (std::exception ex)
		{
			int k = 3;
		}
		std::shared_ptr<IFileRepositoryDB> fileRepDB = CreateFileRepositorySQLiteDB("fileRepo.db", repoPath, true);

		backupDB->StartUpload(fileRepDB);
		backupDB->ContinueUpload();

		SQLite::Statement attach2(*db, "ATTACH DATABASE 'backupDB.db' AS BackupDB");
		attach2.exec();

		try {
			SQLite::Statement copyBackupStateQuery(*db, Text_Resource::CopyBackupState);
			copyBackupStateQuery.bind(":backupID", retValue.id);
			copyBackupStateQuery.exec();
		}
		catch (std::exception ex)
		{
			int k = 3;
		}

		SQLite::Statement detach2(*db, "DETACH DATABASE BackupDB");
		detach2.exec();

		retValue.status = "Complete";
		retValue.ended = std::chrono::system_clock::now();

		SQLite::Statement updateBackup(*db, "UPDATE Backups SET Ended=:ended, Status=:status WHERE ID=:id");
		updateBackup.bind(":ended", get_string_from_time_point(retValue.ended));
		updateBackup.bind(":status", retValue.status);
		updateBackup.bind(":id", retValue.id);
		updateBackup.exec();

		return retValue;
	}

	virtual std::list<BackupInfo> GetBackups() override
	{
		std::list<BackupInfo> retValue;
		SQLite::Statement selectQuery(*db, "SELECT ID, BackupDefID, Started, Ended, Status FROM Backups");

		while (selectQuery.executeStep())
		{
			BackupInfo newValue;
			newValue.id = selectQuery.getColumn("ID").getInt();
			newValue.backupDefId = selectQuery.getColumn("BackupDefID").getInt();
			newValue.started = get_time_point(selectQuery.getColumn("Started").getString());
			newValue.ended = get_time_point(selectQuery.getColumn("Ended").getString());
			newValue.status = selectQuery.getColumn("Status").getString();

			retValue.push_back(newValue);
		}

		return retValue;
	}

	virtual bool Restore(RestoreParameters restoreParams)
	{
		try {
			boost::filesystem::path repoPath("C:\\git\\sbu\\sbu\\SbuCli\\Repo");
			std::shared_ptr<IFileRepositoryDB> fileRepDB = CreateFileRepositorySQLiteDB("fileRepo.db", repoPath, false);

			auto dateToRestoreStr = get_string_from_time_point(restoreParams.dateToRestore);
			SQLite::Statement selectQuery(*db, Text_Resource::RestoreQuery);
			selectQuery.bind(":backupDefID", restoreParams.backupDefId);
			selectQuery.bind(":startDate", dateToRestoreStr);
			while (selectQuery.executeStep())
			{
				auto path = from_utf8(selectQuery.getColumn("Path").getString());
				auto fileHandle = selectQuery.getColumn("FileHandle").getString();

				auto destination = restoreParams.rootDest / path;

				fileRepDB->GetFile(fileHandle, destination);
			}

		}
		catch (std::exception ex)
		{
			int k = 3;
		}
		return true;
	}

	virtual BackupInfo DeleteBackup(Integer backupId) override
	{
		BackupInfo retValue;

		return retValue;
	}

private:
	std::shared_ptr<SQLite::Database> db;
	std::shared_ptr<IFileRepositoryDB> fileDB;
	path root;
	path dbPath;
};

std::shared_ptr<IRepositoryDB> CreateRepositorySQLiteDB(boost::filesystem::path dbPath)
{
	return std::make_shared<RepositoryDB>(dbPath);

}
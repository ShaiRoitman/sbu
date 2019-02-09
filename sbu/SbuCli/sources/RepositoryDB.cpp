#include "RepositoryDB.h"
#include "BackupDB.h"
#include "FileRepositoryDB.h"

#include "SQLiteCpp/SQLiteCpp.h"
#include "utils.h"
#include <iostream>

using namespace boost::filesystem;
using namespace SQLite;

class RepositoryDB : public IRepositoryDB
{
public:
	RepositoryDB(boost::filesystem::path dbPath)
	{
		this->dbPath = dbPath;
		bool dbexists = exists(this->dbPath);
		db = std::make_shared<SQLite::Database>(dbPath.string(), SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);
		if (!dbexists)
		{
			try {
				static std::string createQuery = Text_Resource::Repository;
				db->exec(createQuery);
			}
			catch (std::runtime_error e)
			{
				int k = 3;
			}
		}
	}

	virtual void SetFileRepositoryDB(std::shared_ptr<IFileRepositoryDB> fileDB) override
	{
		this->fileDB = fileDB;
	}

	virtual std::shared_ptr<BackupDef> AddBackupDef(const std::string& name, boost::filesystem::path rootPath) override
	{
		BackupDef retValue;
		try {
			SQLite::Statement insertQuery(*db, "INSERT INTO BackupDefs (Name, Hostname, RootPath, Added) Values (:name, :host, :root, :added)");

			insertQuery.bind(":name", name);
			insertQuery.bind(":host", getHostName());
			insertQuery.bind(":root", to_utf8(rootPath));
			insertQuery.bind(":added", return_current_time_and_date());
			insertQuery.exec();
		}
		catch (std::runtime_error ex)
		{
			std::cout << "Error in adding backup def " + std::string(ex.what()) << std::endl;
		}

		return this->GetBackupDef(name);
	}

	virtual std::shared_ptr<BackupDef> GetBackupDef(const std::string& name)
	{
		SQLite::Statement query(*db, "SELECT ID,Name,Hostname,RootPath,Added FROM BackupDefs WHERE Name=:name");
		query.bind(":name", name);
		std::shared_ptr<BackupDef> retValue = nullptr;
		if (query.executeStep())
		{
			retValue = std::make_shared<BackupDef>();
			retValue->id = query.getColumn("ID").getInt();
			retValue->name = query.getColumn("Name").getString();
			retValue->rootPath = from_utf8(query.getColumn("RootPath").getString());
			retValue->hostName = query.getColumn("Hostname").getInt();
			retValue->added = get_time_point(query.getColumn("Added").getString());

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

	virtual BackupInfo Backup(BackupParameters backupParams, std::shared_ptr<IFileRepositoryDB> fileRepDB) override
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

	virtual std::list<BackupInfo> GetBackups(Integer id) override
	{
		std::list<BackupInfo> retValue;
		SQLite::Statement selectQuery(*db, "SELECT ID, BackupDefID, Started, Ended, Status FROM Backups WHERE BackupDefID=:id");
		selectQuery.bind(":id", id);

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

	virtual bool Restore(RestoreParameters restoreParams, std::shared_ptr<IFileRepositoryDB> fileRepDB)
	{
		try {
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
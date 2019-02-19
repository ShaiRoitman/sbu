#include "RepositoryDB.h"
#include "BackupDB.h"
#include "FileRepositoryDB.h"

#include "SQLiteCpp/SQLiteCpp.h"
#include "utils.h"
#include "sbu_exceptions.h"
#include <iostream>

using namespace boost::filesystem;
using namespace SQLite;

static auto logger = LoggerFactory::getLogger("application.RepositoryDB");

class RepositoryDB : public IRepositoryDB
{
public:
	RepositoryDB(boost::filesystem::path dbPath)
	{
		this->db = getOrCreateDb(dbPath, Text_Resource::Repository);
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
			std::string content = ex.what();
			if (content.find("UNIQUE constraint failed") != std::string::npos)
			{
				throw sbu_alreadyexists();
			}
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
			retValue->hostName = query.getColumn("Hostname").getString();
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
		BackupDef backupDef;
		try {
			retValue = CreateBackupInfo(backupParams, backupDef);

			auto backupDB = getBackupDB();
			CalculateCurrentStatus(backupDB, backupDef);

			UpdatedBackup("CalculatingDiff", retValue);
			backupDB->StartDiffCalc();
			backupDB->ContinueDiffCalc();

			UpdatedBackup("Uploading", retValue);
			backupDB->StartUpload(fileRepDB);
			backupDB->ContinueUpload(fileRepDB);

			UpdatedBackup("UpdatingRepository", retValue);
			CopyBackupToRepository(retValue);

			UpdatedBackup("Complete", retValue);

		}
		catch (std::exception ex)
		{
			UpdatedBackup("Failed", retValue);
			logger->ErrorFormat("RepositoryDB::Backup() Error with exception:[%s]", ex.what());
		}
		return retValue;
	}

	virtual std::list<BackupInfo> GetBackups(Integer id) override
	{
		std::list<BackupInfo> retValue;
		SQLite::Statement selectQuery(*db, "SELECT ID, BackupDefID, Started, LastStatusUpdate, Status FROM Backups WHERE BackupDefID=:id");
		selectQuery.bind(":id", id);

		while (selectQuery.executeStep())
		{
			BackupInfo newValue;
			newValue.id = selectQuery.getColumn("ID").getInt();
			newValue.backupDefId = selectQuery.getColumn("BackupDefID").getInt();
			newValue.started = get_time_point(selectQuery.getColumn("Started").getString());
			newValue.lastUpdated = get_time_point(selectQuery.getColumn("LastStatusUpdate").getString());
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
				auto type = selectQuery.getColumn("Type").getString();
				auto path = from_utf8(selectQuery.getColumn("Path").getString());
				auto fileHandle = selectQuery.getColumn("FileHandle").getString();

				auto destination = restoreParams.rootDest / path;
				if (type == "File")
				{
					fileRepDB->GetFile(fileHandle, destination);
				}
				else
				{
					boost::filesystem::create_directories(destination);
				}
			}

		}
		catch (std::exception ex)
		{
			logger->Error((std::string("Error in ") + std::string(ex.what())).c_str());
		}

		return true;
	}

	virtual BackupInfo DeleteBackup(Integer backupId) override
	{
		BackupInfo retValue;

		return retValue;
	}

private:
	BackupInfo CreateBackupInfo(const BackupParameters& backupParams, BackupDef& backupDef)
	{
		BackupInfo retValue;

		retValue.backupDefId = backupParams.backupDefId;
		retValue.started = std::chrono::system_clock::now();
		retValue.status = "Started";

		SQLite::Statement insertBackup(*db, "INSERT INTO Backups (BackupDefID, Started, , LastStatusUpdate, Status ) VALUES ( :backupdefID, :started, :lastUpdated, :status )");
		insertBackup.bind(":backupdefID", retValue.backupDefId);
		insertBackup.bind(":started", return_current_time_and_date());
		insertBackup.bind(":lastUpdated", return_current_time_and_date());
		insertBackup.bind(":status", "Started");
		if (insertBackup.exec() > 0)
		{
			SQLite::Statement selectBackup(*db, Text_Resource::findBackupInfo);
			selectBackup.bind(":backupdefID", retValue.backupDefId);
			if (selectBackup.executeStep())
			{
				retValue.id = selectBackup.getColumn("ID").getInt();
				backupDef.id = selectBackup.getColumn("BackupDefID").getInt();
				backupDef.rootPath = from_utf8(selectBackup.getColumn("RootPath").getString());
			}
		}

		return retValue;
	}

	void CalculateCurrentStatus(std::shared_ptr<IBackupDB> backupDB, const BackupDef& backupDef)
	{
		backupDB->StartScan(backupDef.rootPath);

		SQLite::Statement attach(*db, "ATTACH DATABASE 'backupDB.db' AS BackupDB");
		attach.exec();

		SQLite::Statement currentStateQuery(*db, Text_Resource::CurrentState);
		currentStateQuery.bind(":backupDefID", backupDef.id);
		currentStateQuery.exec();

		SQLite::Statement detach(*db, "DETACH DATABASE BackupDB");
		detach.exec();
	}

	void CopyBackupToRepository(const BackupInfo& retValue)
	{
		SQLite::Statement attach2(*db, "ATTACH DATABASE 'backupDB.db' AS BackupDB");
		attach2.exec();

		SQLite::Statement copyBackupStateQuery(*db, Text_Resource::CopyBackupState);
		copyBackupStateQuery.bind(":backupID", retValue.id);
		copyBackupStateQuery.exec();

		SQLite::Statement detach2(*db, "DETACH DATABASE BackupDB");
		detach2.exec();
	}

	void UpdatedBackup(const std::string& Status, BackupInfo& retValue)
	{
		retValue.status = Status;
		retValue.lastUpdated = std::chrono::system_clock::now();

		SQLite::Statement updateBackup(*db, "UPDATE Backups SET LastStatusUpdate=:lastUpdated, Status=:status WHERE ID=:id");
		updateBackup.bind(":lastUpdated", get_string_from_time_point(retValue.lastUpdated));
		updateBackup.bind(":status", retValue.status);
		updateBackup.bind(":id", retValue.id);
		updateBackup.exec();
	}

	std::shared_ptr<IBackupDB> getBackupDB()
	{
		boost::filesystem::remove("backupDB.db");
		std::shared_ptr<IBackupDB> backupDB = CreateSQLiteDB("backupDB.db");

		return backupDB;
	}

	std::shared_ptr<SQLite::Database> db;
	std::shared_ptr<IFileRepositoryDB> fileDB;
	path root;
};

std::shared_ptr<IRepositoryDB> CreateRepositorySQLiteDB(boost::filesystem::path dbPath)
{
	logger->DebugFormat("Creating RepositoryDB dbPath:[%s]", dbPath.string().c_str());
	return std::make_shared<RepositoryDB>(dbPath);

}
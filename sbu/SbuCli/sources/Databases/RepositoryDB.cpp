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

	void LogBackupDef(const std::string& prefix,std::shared_ptr<BackupDef> def)
	{
		if (def != nullptr)
		{
			logger->DebugFormat("[%s] ID:[%d] Name:[%s] RootPath:[%s] Host:[%s] Added:[%s]",
				prefix.c_str(),
				def->id,
				def->name.c_str(),
				def->rootPath.string().c_str(),
				def->hostName.c_str(),
				get_string_from_time_point(def->added).c_str()
			);
		}
	}

	virtual std::shared_ptr<BackupDef> AddBackupDef(const std::string& name, boost::filesystem::path rootPath) override
	{
		logger->DebugFormat("Adding BackupDef Name:[%s] RootPath:[%s]", name.c_str(), rootPath.string().c_str());
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
				logger->ErrorFormat("RepositoryDB::AddBackupDef() Failed due to uniqueness");
				throw sbu_alreadyexists();
			}
			logger->ErrorFormat("RepositoryDB::AddBackupDef() Failed");
			std::cout << "Error in adding backup def " + std::string(ex.what()) << std::endl;
		}

		auto retValue = this->GetBackupDef(name);
		LogBackupDef("RepositoryDB::AddBackupDef() Added ", retValue);
		return retValue;
	}

	virtual std::shared_ptr<BackupDef> GetBackupDef(Integer id)
	{
		SQLite::Statement query(*db, "SELECT ID,Name,Hostname,RootPath,Added FROM BackupDefs WHERE ID=:id");
		query.bind(":id", id);
		std::shared_ptr<BackupDef> retValue = nullptr;
		if (query.executeStep())
		{
			retValue = std::make_shared<BackupDef>();
			retValue->id = query.getColumn("ID").getInt();
			retValue->name = query.getColumn("Name").getString();
			retValue->rootPath = from_utf8(query.getColumn("RootPath").getString());
			retValue->hostName = query.getColumn("Hostname").getString();
			retValue->added = get_time_point(query.getColumn("Added").getString());

			logger->InfoFormat("RepositoryDB::GetBackupDef(Integer) name:[%s] ID:[%ld] RootPath:[%s] Hostname:[%s] Added:[%s]",
				retValue->name.c_str(),
				retValue->id,
				retValue->rootPath.string().c_str(),
				retValue->hostName.c_str(),
				get_string_from_time_point(retValue->added).c_str());

		}

		LogBackupDef("RepositoryDB::GetBackupDef(Integer) Get ", retValue);

		return retValue;
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

			logger->InfoFormat("RepositoryDB::GetBackupDef(string) name:[%s] ID:[%ld] RootPath:[%s] Hostname:[%s] Added:[%s]",
				name.c_str(),
				retValue->id,
				retValue->rootPath.string().c_str(),
				retValue->hostName.c_str(),
				get_string_from_time_point(retValue->added).c_str());

		}

		LogBackupDef("RepositoryDB::GetBackupDef(string) Get ", retValue);

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

	virtual void ListBackupDefs(std::function<void(const IRepositoryDB::BackupDef& backupdef)> iter) override
	{
		SQLite::Statement selectBackupDefs(*db, "SELECT ID, Name, Hostname, RootPath, Added FROM BackupDefs");

		while (selectBackupDefs.executeStep())
		{
			BackupDef newValue;
			newValue.id = selectBackupDefs.getColumn("ID").getInt();
			newValue.name = selectBackupDefs.getColumn("Name").getString();
			newValue.hostName = selectBackupDefs.getColumn("Hostname").getString();
			newValue.rootPath = from_utf8(selectBackupDefs.getColumn("RootPath").getString());
			newValue.added = get_time_point(selectBackupDefs.getColumn("Added").getString());

			iter(newValue);
		}
	}

	virtual BackupInfo Backup(BackupParameters backupParams, std::shared_ptr<IFileRepositoryDB> fileRepDB) override
	{
		BackupInfo retValue;
		BackupDef backupDef;
		backupDef.id = backupParams.backupDefId;
		try {
			retValue = CreateBackupInfo(backupDef);

			UpdatedBackup("Starting Backup", retValue);
			boost::filesystem::remove("backupDB.db");
			auto backupDB = getBackupDB();

			UpdatedBackup("Scanning FileSystem", retValue);
			backupDB->StartScan(backupDef.rootPath);
			backupDB->ContinueScan();
			this->CopyCurrentStateIntoBackupDB("backupDB.db", backupDef);

			UpdatedBackup("CalculatingDiff", retValue);
			backupDB->StartDiffCalc();
			backupDB->ContinueDiffCalc();

			UpdatedBackup("Uploading", retValue);
			backupDB->StartUpload(fileRepDB);
			backupDB->ContinueUpload(fileRepDB);
			backupDB->Complete();

			UpdatedBackup("UpdatingRepository", retValue);
			CopyBackupDBStateIntoRepoAndComplete("backupDB.db", retValue);
		}
		catch (std::exception ex)
		{
			UpdatedBackup("Failed", retValue);
			logger->ErrorFormat("RepositoryDB::Backup() Error with exception:[%s]", ex.what());
		}
		return retValue;
	}

	virtual void ListBackups(Integer id, std::function<void(const IRepositoryDB::BackupInfo& backup)> function) override
	{
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
			function(newValue);
		}
	}

	virtual void ListBackupInfo(Integer id,
		std::function<
		void(const std::string& status,
			const std::string& path,
			const std::string& type)> function) override
	{
		SQLite::Statement selectQuery(*db, "SELECT Status, Path, Type FROM Files WHERE BackupID=:id ORDER BY Status, Path  ");
		selectQuery.bind(":id", id);
		while (selectQuery.executeStep())
		{
			function(
				selectQuery.getColumn("Status").getString(),
				selectQuery.getColumn("Path").getString(),
				selectQuery.getColumn("Type").getString());
		}
	}

	virtual bool Restore(RestoreParameters restoreParams, std::shared_ptr<IFileRepositoryDB> fileRepDB)
	{
		try {
			auto dateToRestoreStr = get_string_from_time_point(restoreParams.dateToRestore);
			SQLite::Statement selectQuery(*db, Text_Resource::RestoreQuery);
			selectQuery.bind(":backupDefID", restoreParams.backupDefId);
			selectQuery.bind(":startDate", dateToRestoreStr);
			selectQuery.bind(":backupID", restoreParams.byID);
			while (selectQuery.executeStep())
			{
				auto type = selectQuery.getColumn("Type").getString();
				auto path = from_utf8(selectQuery.getColumn("Path").getString());
				auto fileHandle = selectQuery.getColumn("FileHandle").getString();

				auto destination = restoreParams.rootDest / path;
				if (type == "File")
				{
					if (restoreParams.shouldCopy)
					{
						fileRepDB->GetFile(fileHandle, destination);
					}
					else
					{
						if (restoreParams.altToCopyFunc != nullptr)
						{
							restoreParams.altToCopyFunc(destination);
						}
					}
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

	virtual BackupInfo CreateBackupInfo(BackupDef& backupDef) override
	{
		BackupInfo retValue;

		retValue.backupDefId = backupDef.id;
		retValue.started = std::chrono::system_clock::now();
		retValue.status = "Started";

		SQLite::Statement insertBackup(*db, "INSERT INTO Backups (BackupDefID, Started, LastStatusUpdate, Status ) VALUES ( :backupdefID, :started, :lastUpdated, :status )");
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

	void CopyCurrentStateIntoBackupDB(boost::filesystem::path backupDBPath, const BackupDef& backupDef) override
	{
		SQLite::Statement attach(*db, "ATTACH DATABASE :backupDB AS BackupDB");
		attach.bind(":backupDB", backupDBPath.string().c_str());
		attach.exec();

		SQLite::Statement currentStateQuery(*db, Text_Resource::CurrentState);
		currentStateQuery.bind(":backupDefID", backupDef.id);
		currentStateQuery.exec();

		SQLite::Statement detach(*db, "DETACH DATABASE BackupDB");
		detach.exec();
	}

private:
	void CopyBackupDBStateIntoRepoAndComplete(boost::filesystem::path backupDBPath, BackupInfo& retValue) override
	{
		SQLite::Statement attach(*db, "ATTACH DATABASE :backupDB AS BackupDB");
		attach.bind(":backupDB", backupDBPath.string().c_str());
		attach.exec();

		SQLite::Statement copyBackupStateQuery(*db, Text_Resource::CopyBackupState);
		copyBackupStateQuery.bind(":backupID", retValue.id);
		copyBackupStateQuery.exec();

		SQLite::Statement detach(*db, "DETACH DATABASE BackupDB");
		detach.exec();

		UpdatedBackup("Complete", retValue);

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
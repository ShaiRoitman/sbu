#include "RepositoryDB.h"
#include "BackupDB.h"
#include "FileRepositoryDB.h"

#include "utils.h"
#include "sbu_exceptions.h"
#include <iostream>
#include "boost/format.hpp"
#include "boost/lexical_cast.hpp"

using namespace boost::filesystem;

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
		AddToExecutionLog(db, "AddBackupDef()", name);
		try {
			auto insertQuery = db->CreateStatement("INSERT INTO BackupDefs (Name, Hostname, RootPath, Added) VALUES (:name, :host, :root, :added)");

			insertQuery->bind(":name", name);
			insertQuery->bind(":host", getHostName());
			insertQuery->bind(":root", to_utf8(rootPath));
			insertQuery->bind(":added", return_current_time_and_date());
			insertQuery->exec();
		}
		catch (std::runtime_error ex)
		{
			std::string content = ex.what();
			if (content.find("UNIQUE constraint failed") != std::string::npos)
			{
				AddToExecutionLog(db, "CreateBackupDef() Failed Uniqueness", (boost::format("name:[%1%] path:[%2%]") % name % rootPath).str());
				logger->ErrorFormat("RepositoryDB::AddBackupDef() Failed due to uniqueness");
				throw sbu_alreadyexists();
			}
			AddToExecutionLog(db, "CreateBackupDef() Failed", (boost::format("name:[%1%] path:[%2%]") % name % rootPath).str());
			logger->ErrorFormat("RepositoryDB::AddBackupDef() Failed");
			std::cout << "Error in adding backup def " + std::string(ex.what()) << std::endl;
		}

		auto retValue = this->GetBackupDef(name);
		LogBackupDef("RepositoryDB::AddBackupDef() Added ", retValue);
		AddToExecutionLog(db, "CreateBackupDef() Success", (boost::format("name:[%1%] path:[%2%]") % name % rootPath).str()) ;
		return retValue;
	}

	virtual std::shared_ptr<BackupDef> GetBackupDef(Integer id)
	{
		auto query = db->CreateStatement("SELECT ID,Name,Hostname,RootPath,Added FROM BackupDefs WHERE ID=:id");
		query->bind(":id", id);
		std::shared_ptr<BackupDef> retValue = nullptr;
		if (query->executeStep())
		{
			retValue = std::make_shared<BackupDef>();
			retValue->id = query->getColumn("ID")->getInt();
			retValue->name = query->getColumn("Name")->getString();
			retValue->rootPath = from_utf8(query->getColumn("RootPath")->getString());
			retValue->hostName = query->getColumn("Hostname")->getString();
			retValue->added = get_time_point(query->getColumn("Added")->getString());

			logger->InfoFormat("RepositoryDB::GetBackupDef(Integer) name:[%s] ID:[%ld] RootPath:[%s] Hostname:[%s] Added:[%s]",
				retValue->name.c_str(),
				retValue->id,
				retValue->rootPath.string().c_str(),
				retValue->hostName.c_str(),
				get_string_from_time_point(retValue->added).c_str());

		}

		LogBackupDef("RepositoryDB::GetBackupDef(Integer) Get ", retValue);

		logger->DebugFormat("RepositoryDB::GetBackupDef() id:[%d]", id);

		return retValue;
	}

	virtual std::shared_ptr<BackupDef> GetBackupDef(const std::string& name)
	{
		auto query = db->CreateStatement("SELECT ID,Name,Hostname,RootPath,Added FROM BackupDefs WHERE Name=:name");
		query->bind(":name", name);
		std::shared_ptr<BackupDef> retValue = nullptr;
		if (query->executeStep())
		{
			retValue = std::make_shared<BackupDef>();
			retValue->id = query->getColumn("ID")->getInt();
			retValue->name = query->getColumn("Name")->getString();
			retValue->rootPath = from_utf8(query->getColumn("RootPath")->getString());
			retValue->hostName = query->getColumn("Hostname")->getString();
			retValue->added = get_time_point(query->getColumn("Added")->getString());

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
		AddToExecutionLog(db, "DeleteBackupDef()", boost::lexical_cast<std::string>(id));
		auto transaction = db->CreateTransaction();

		auto deleteBackups = db->CreateStatement("DELETE FROM Backups WHERE BackupDefID = :id");
		deleteBackups->bind(":id", id);
		deleteBackups->exec();

		auto deleteBackupDefs =db->CreateStatement("DELETE FROM BackupDefs WHERE ID= :id");
		deleteBackupDefs->bind(":id", id);
		bool retValue = deleteBackupDefs->exec() > 0;

		transaction->commit();

		AddToExecutionLog(db, "DeleteBackupDef() Success", (boost::format("id:[%1%]") % id).str());

		logger->DebugFormat("RepositoryDB::DeleteBackupDef() id:[%d]", id);
		return retValue;
	}

	virtual void ListBackupDefs(std::function<void(const IRepositoryDB::BackupDef& backupdef)> iter) override
	{
		auto selectBackupDefs = db->CreateStatement("SELECT ID, Name, Hostname, RootPath, Added FROM BackupDefs");

		while (selectBackupDefs->executeStep())
		{
			BackupDef newValue;
			newValue.id = selectBackupDefs->getColumn("ID")->getInt();
			newValue.name = selectBackupDefs->getColumn("Name")->getString();
			newValue.hostName = selectBackupDefs->getColumn("Hostname")->getString();
			newValue.rootPath = from_utf8(selectBackupDefs->getColumn("RootPath")->getString());
			newValue.added = get_time_point(selectBackupDefs->getColumn("Added")->getString());

			logger->DebugFormat("RepositoryDB::ListBackupDefs() id:[%d] name:[%s] hostname:[%s] rootPath:[%s]",
				newValue.id,
				newValue.name.c_str(),
				newValue.hostName.c_str(),
				newValue.rootPath.c_str());

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
			const std::string fileName = "BackupDB.db";

			UpdatedBackup("Starting Backup", retValue);
			removeFile(fileName);
			auto backupDB = getBackupDB();

			UpdatedBackup("Scanning FileSystem", retValue);
			backupDB->StartScan(backupDef.rootPath);
			backupDB->ContinueScan();
			this->CopyCurrentStateIntoBackupDB(fileName, backupDef);

			UpdatedBackup("CalculatingDiff", retValue);
			backupDB->StartDiffCalc();
			backupDB->ContinueDiffCalc();

			UpdatedBackup("Uploading", retValue);
			backupDB->StartUpload(fileRepDB);
			backupDB->ContinueUpload(fileRepDB);
			backupDB->Complete();

			UpdatedBackup("UpdatingRepository", retValue);
			CopyBackupDBStateIntoRepoAndComplete(fileName, retValue);
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
		auto selectQuery = db->CreateStatement("SELECT ID, BackupDefID, Started, LastStatusUpdate, Status FROM Backups WHERE BackupDefID=:id");
		selectQuery->bind(":id", id);

		while (selectQuery->executeStep())
		{
			BackupInfo newValue;
			newValue.id = selectQuery->getColumn("ID")->getInt();
			newValue.backupDefId = selectQuery->getColumn("BackupDefID")->getInt();
			newValue.started = get_time_point(selectQuery->getColumn("Started")->getString());
			newValue.lastUpdated = get_time_point(selectQuery->getColumn("LastStatusUpdate")->getString());
			newValue.status = selectQuery->getColumn("Status")->getString();

			logger->DebugFormat("RepositoryDB::ListBackups() id:[%d] backupDefId:[%d] status:[%s]",
				newValue.id,
				newValue.backupDefId,
				newValue.status.c_str());

			function(newValue);
		}
	}

	virtual void ListBackupInfo(Integer id,
		std::function<
		void(const std::string& status,
			const std::string& path,
			const std::string& type)> function) override
	{
		auto selectQuery = db->CreateStatement("SELECT Status, Path, Type FROM Files WHERE BackupID=:id ORDER BY Status, Path  ");
		selectQuery->bind(":id", id);
		auto status = selectQuery->getColumn("Status")->getString();
		auto path = selectQuery->getColumn("Path")->getString();
		auto type = selectQuery->getColumn("Type")->getString();

		while (selectQuery->executeStep())
		{
			logger->DebugFormat("RepositoryDB::ListBackupInfo() Status:[%d] Path:[%s] Type:[%s]", status.c_str(), path.c_str(), type.c_str());
			function(status, path, type);
		}
	}

	virtual bool Restore(RestoreParameters restoreParams, std::shared_ptr<IFileRepositoryDB> fileRepDB)
	{
		try {
			auto dateToRestoreStr = get_string_from_time_point(restoreParams.dateToRestore);
			auto selectQuery = db->CreateStatement(Text_Resource::RestoreQuery);
			selectQuery->bind(":backupDefID", restoreParams.backupDefId);
			selectQuery->bind(":startDate", dateToRestoreStr);
			selectQuery->bind(":backupID", restoreParams.byID);
			while (selectQuery->executeStep())
			{
				auto type = selectQuery->getColumn("Type")->getString();
				auto path = from_utf8(selectQuery->getColumn("Path")->getString());
				auto fileHandle = selectQuery->getColumn("FileHandle")->getString();
				auto destination = restoreParams.rootDest / path;

				logger->DebugFormat("RepositoryDB::Restore() path:[%s] fileHandle:[%s] destination:[%s]", path.c_str(), fileHandle.c_str(), destination.c_str());
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

		auto insertBackup = db->CreateStatement("INSERT INTO Backups (BackupDefID, Started, LastStatusUpdate, Status ) VALUES ( :backupdefID, :started, :lastUpdated, :status )");
		insertBackup->bind(":backupdefID", retValue.backupDefId);
		insertBackup->bind(":started", return_current_time_and_date());
		insertBackup->bind(":lastUpdated", return_current_time_and_date());
		insertBackup->bind(":status", "Started");
		if (insertBackup->exec() > 0)
		{
			auto selectBackup = db->CreateStatement(Text_Resource::findBackupInfo);
			selectBackup->bind(":backupdefID", retValue.backupDefId);
			if (selectBackup->executeStep())
			{
				retValue.id = selectBackup->getColumn("ID")->getInt();
				backupDef.id = selectBackup->getColumn("BackupDefID")->getInt();
				backupDef.rootPath = from_utf8(selectBackup->getColumn("RootPath")->getString());
			}
		}

		return retValue;
	}

	void CopyCurrentStateIntoBackupDB(boost::filesystem::path backupDBPath, const BackupDef& backupDef) override
	{
		logger->DebugFormat("RepositoryDB::CopyCurrentStateIntoBackupDB() backupDBPath:[%s] Attaching", backupDBPath.c_str());
		auto attach= db->CreateStatement("ATTACH DATABASE :backupDB AS BackupDB");
		attach->bind(":backupDB", backupDBPath.string().c_str());
		attach->exec();

		auto currentStateQuery = db->CreateStatement(Text_Resource::CurrentState);
		currentStateQuery->bind(":backupDefID", backupDef.id);
		currentStateQuery->exec();

		logger->DebugFormat("RepositoryDB::CopyCurrentStateIntoBackupDB() backupDBPath:[%s] Detaching", backupDBPath.c_str());
		auto detach = db->CreateStatement("DETACH DATABASE BackupDB");
		detach->exec();
	}

private:
	void CopyBackupDBStateIntoRepoAndComplete(boost::filesystem::path backupDBPath, BackupInfo& retValue) override
	{
		logger->DebugFormat("RepositoryDB::CopyBackupDBStateIntoRepoAndComplete() backupDBPath:[%s] Attaching", backupDBPath.c_str());
		auto attach =db->CreateStatement("ATTACH DATABASE :backupDB AS BackupDB");
		attach->bind(":backupDB", backupDBPath.string().c_str());
		attach->exec();

		auto copyBackupStateQuery = db->CreateStatement(Text_Resource::CopyBackupState);
		copyBackupStateQuery->bind(":backupID", retValue.id);
		copyBackupStateQuery->exec();

		logger->DebugFormat("RepositoryDB::CopyBackupDBStateIntoRepoAndComplete() backupDBPath:[%s] Detaching", backupDBPath.c_str());
		auto detach = db->CreateStatement("DETACH DATABASE BackupDB");
		detach->exec();

		UpdatedBackup("Complete", retValue);

	}

	void UpdatedBackup(const std::string& status, BackupInfo& retValue)
	{
		retValue.status = status;
		retValue.lastUpdated = std::chrono::system_clock::now();

		auto updateBackup = db->CreateStatement("UPDATE Backups SET LastStatusUpdate=:lastUpdated, Status=:status WHERE ID=:id");
		updateBackup->bind(":lastUpdated", get_string_from_time_point(retValue.lastUpdated));
		updateBackup->bind(":status", retValue.status);
		updateBackup->bind(":id", retValue.id);
		updateBackup->exec();
		logger->DebugFormat("RepositoryDB::UpdatedBackup() Status:[%s]", status.c_str());
	}

	std::shared_ptr<IBackupDB> getBackupDB()
	{
		std::shared_ptr<IBackupDB> backupDB = CreateDB("backupDB.db");

		return backupDB;
	}

	std::shared_ptr<ISbuDBDatabase> db;
	std::shared_ptr<IFileRepositoryDB> fileDB;
	path root;
};

std::shared_ptr<IRepositoryDB> CreateRepositoryDB(boost::filesystem::path dbPath)
{
	logger->DebugFormat("Creating RepositoryDB dbPath:[%s]", dbPath.string().c_str());
	return std::make_shared<RepositoryDB>(dbPath);
}
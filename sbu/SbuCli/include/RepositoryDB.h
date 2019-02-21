#pragma once

#include <boost/filesystem.hpp>
#include <string>
#include <chrono>
#include "FileRepositoryDB.h"
#include "utils.h"

class IRepositoryDB
{
public:
	class BackupDef
	{
	public:
		Integer id;
		std::string name;
		std::string hostName;
		boost::filesystem::path rootPath;
		std::chrono::system_clock::time_point added;
	};

	class BackupInfo
	{
	public:
		Integer id;
		Integer backupDefId;
		std::chrono::system_clock::time_point started;
		std::chrono::system_clock::time_point lastUpdated;
		std::string status;
	};

	class BackupParameters
	{
	public:
		BackupParameters& BackupDefId(Integer bid)
		{
			this->backupDefId = bid;
			return *this;
		}

		Integer backupDefId;
	};

	class RestoreParameters
	{
	public:
		RestoreParameters()
		{
			this->dateToRestore = std::chrono::system_clock::now();
			this->byID = INT_MAX;
			this->shouldCopy = true;
		}

		RestoreParameters& BackupDefId(Integer bid)
		{
			this->backupDefId = bid;
			return *this;
		}

		RestoreParameters& RootDest(const std::string& root)
		{
			this->rootDest = root;
			return *this;
		}

		RestoreParameters& DateToRestore(std::chrono::system_clock::time_point date)
		{
			this->dateToRestore = date;
			return *this;
		}

		RestoreParameters& ShowOnly(bool showOnly)
		{
			this->shouldCopy = !showOnly;
			return *this;
		}

		RestoreParameters& ShowOnly(Integer byID)
		{
			this->byID= byID;
			return *this;
		}

		Integer backupDefId;
		boost::filesystem::path rootDest;
		std::chrono::system_clock::time_point dateToRestore;
		bool shouldCopy;
		Integer byID;
	};

	virtual std::shared_ptr<BackupDef> AddBackupDef(const std::string& name, boost::filesystem::path rootPath) = 0;
	virtual std::shared_ptr<BackupDef> GetBackupDef(const std::string& name) = 0;
	virtual bool DeleteBackupDef(Integer id) = 0;
	virtual std::list<BackupDef> GetBackupDefs() = 0;

	virtual BackupInfo Backup(BackupParameters backupParams, std::shared_ptr<IFileRepositoryDB> fileRepDB) = 0;
	virtual void ListBackups(Integer id) = 0;
	virtual BackupInfo DeleteBackup(Integer backupId) = 0;
	virtual void ListBackupInfo(Integer id) = 0;

	virtual bool Restore(RestoreParameters restoreParams, std::shared_ptr<IFileRepositoryDB> fileRepDB) = 0;

	virtual void SetFileRepositoryDB(std::shared_ptr<IFileRepositoryDB> fileDB) = 0;
};

std::shared_ptr<IRepositoryDB> CreateRepositorySQLiteDB(boost::filesystem::path dbPath);
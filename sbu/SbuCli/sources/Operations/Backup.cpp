#include "Operations.h"

#include <iostream>
#include "utils.h"
#include "factories.h"
#include "sbu_exceptions.h"
#include "ExitCodes.h"

class BackupOperation : public Operation
{
public:
	BackupOperation() : logger(LoggerFactory::getLogger("Operations")) {}
	int Init(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;
		std::string dbPath = vm["BackupDB.path"].as<std::string>();
		bool doesExists = boost::filesystem::exists(dbPath);

		if (doesExists == false)
		{
			std::string name = vm["name"].as<std::string>();
			auto RepoDB = getRepository(vm);
			auto backupdef = RepoDB->GetBackupDef(name);
			if (backupdef != nullptr)
			{
				auto backupDB = CreateSQLiteDB(dbPath);
				backupDB->StartScan(backupdef->rootPath);
				RepoDB->CopyCurrentStateIntoBackupDB(dbPath, *backupdef);

			}
			else
			{
				retValue = ExitCode_GeneralFailure;
			}

		}
		else
		{
			retValue = ExitCode_AlreadyExists;
		}

		logger->DebugFormat("Operation:[Backup] DbPath:[%s] Step [Init] retValue:[%d]", dbPath.c_str(), retValue);

		return retValue;
	}
	
	int Scan(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;

		std::string dbPath = vm["BackupDB.path"].as<std::string>();

		logger->DebugFormat("Operation:[Scan] dbPath:[%s]", dbPath.c_str());

		bool doesExists = boost::filesystem::exists(dbPath);
		if (doesExists)
		{
			auto backupDB = CreateSQLiteDB(dbPath);
			backupDB->ContinueScan();
		}
		else
		{
			retValue = ExitCode_GeneralFailure;		}

		logger->DebugFormat("Operation:[Scan] dbPath:[%s] retValue:[%d]", dbPath.c_str(), retValue);
		return retValue;
	}

	int DiffCalc(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;

		std::string dbPath = vm["BackupDB.path"].as<std::string>();

		logger->DebugFormat("Operation:[DiffCalc] dbPath:[%s]", dbPath.c_str());

		bool doesExists = boost::filesystem::exists(dbPath);
		if (doesExists)
		{
			auto backupDB = CreateSQLiteDB(dbPath);
			backupDB->StartDiffCalc();
			backupDB->ContinueDiffCalc();
		}
		else
		{
			retValue = ExitCode_GeneralFailure;
		}

		logger->DebugFormat("Operation:[DiffCalc] dbPath:[%s] retValue:[%d]", dbPath.c_str(), retValue);
		return retValue;
	}

	int FileUpload(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;

		std::string dbPath = vm["BackupDB.path"].as<std::string>();

		logger->DebugFormat("Operation:[FileUpload] dbPath:[%s]", dbPath.c_str());

		bool doesExists = boost::filesystem::exists(dbPath);
		if (doesExists)
		{
			auto backupDB = CreateSQLiteDB(dbPath);
			std::shared_ptr<IFileRepositoryDB> fileRepDB = getFileRepository(vm);
			backupDB->StartUpload(fileRepDB);
			backupDB->ContinueUpload(fileRepDB);
		}
		else
		{
			retValue = ExitCode_GeneralFailure;
		}

		logger->DebugFormat("Operation:[FileUpload] dbPath:[%s] retValue:[%d]", dbPath.c_str(), retValue);
		return retValue;
	}

	int Complete(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;

		std::string dbPath = vm["BackupDB.path"].as<std::string>();

		logger->DebugFormat("Operation:[Complete] dbPath:[%s]", dbPath.c_str());

		bool doesExists = boost::filesystem::exists(dbPath);
		if (doesExists)
		{
			auto backupDB = CreateSQLiteDB(dbPath);
			backupDB->ContinueScan();
			backupDB->ContinueDiffCalc();
			std::shared_ptr<IFileRepositoryDB> fileRepDB = getFileRepository(vm);
			backupDB->ContinueUpload(fileRepDB);
			backupDB->Complete();

			std::string name = vm["name"].as<std::string>();
			auto RepoDB = getRepository(vm);
			auto backupdef = RepoDB->GetBackupDef(name);

			auto backupInfo = RepoDB->CreateBackupInfo(*backupdef);
			RepoDB->CopyBackupDBStateIntoRepoAndComplete(dbPath, backupInfo);
		}
		else
		{
			retValue = ExitCode_GeneralFailure;
		}

		logger->DebugFormat("Operation:[Complete] dbPath:[%s] retValue:[%d]", dbPath.c_str(), retValue);
		return retValue;
	}

	int All(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;

		std::string name = vm["name"].as<std::string>();

		logger->DebugFormat("Operation:[Backup] Name:[%s] Step [All]", name.c_str());

		auto RepoDB = getRepository(vm);
		auto backupdef = RepoDB->GetBackupDef(name);
		if (backupdef != nullptr)
		{
			std::shared_ptr<IFileRepositoryDB> fileRepDB = getFileRepository(vm);
			auto backupInfo = RepoDB->Backup(IRepositoryDB::BackupParameters().BackupDefId(backupdef->id), fileRepDB);
		}

		logger->DebugFormat("Operation:[Backup] Name:[%s] Step [All] retValue:[%d]", name.c_str(), retValue);
		return retValue;
	}

	int Operate(boost::program_options::variables_map& vm)
	{
		int retValue = ExitCode_Success;
		std::string step = "All";
		if (!vm["bstep"].empty())
		{
			step = vm["bstep"].as<std::string>();
		}

		logger->DebugFormat("Operation:[Backup] Step [%s]", step.c_str());

		if (step == "All")
			retValue = this->All(vm);
		else if (step == "Init")
			retValue = this->Init(vm);
		else if (step == "Scan")
			retValue = this->Scan(vm);
		else if (step == "DiffCalc")
			retValue = this->DiffCalc(vm);
		else if (step == "FileUpload")
			retValue = this->FileUpload(vm);
		else if (step == "Complete")
			retValue = this->Complete(vm);

		logger->DebugFormat("Operation:[Backup] Step [%s] retValue:[%d]", step.c_str(), retValue);
		return retValue;
	}

private:
	std::shared_ptr<ILogger> logger;
};

std::shared_ptr<Operation> BackupFactory()
{
	return std::make_shared<BackupOperation>();
}

#include "Operations.h"

#include <iostream>
#include "utils.h"
#include "factories.h"
#include "sbu_exceptions.h"
#include "ExitCodes.h"

#include "Operations/BackupOperation.h"

static auto logger = LoggerFactory::getLogger("Operations.Backup");

BackupOperation::BackupOperation() 
{
}

BackupOperation::BackupOperation(std::shared_ptr<Strategy> strategy)
{
	this->strategy = strategy;
}

int BackupOperation::Init(boost::program_options::variables_map& vm)
{
	int retValue = ExitCode_Success;
	std::string dbPath = getValueAsString(vm, "BackupDB.path");
	bool doesExists = boost::filesystem::exists(dbPath);
	if (doesExists == false)
	{
		std::string name = getValueAsString(vm, "name");
		auto RepoDB = getRepository(vm);
		auto backupdef = RepoDB->GetBackupDef(name);
		if (backupdef != nullptr)
		{
			auto backupDB = CreateDB(dbPath);
			backupDB->StartScan(backupdef->rootPath);
			boost::filesystem::path backupDBPath = getValueAsString(vm, "BackupDB.path");
			RepoDB->CopyCurrentStateIntoBackupDB(*backupdef, backupDBPath);
		}
		else
		{
			logger->DebugFormat("Operation:[Backup] Failed due to parameter name not provided");
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

int BackupOperation::Scan(boost::program_options::variables_map& vm)
{
	int retValue = ExitCode_Success;

	std::string dbPath = getValueAsString(vm, "BackupDB.path");

	logger->DebugFormat("Operation:[Scan] dbPath:[%s]", dbPath.c_str());

	bool doesExists = boost::filesystem::exists(dbPath);
	if (doesExists)
	{
		auto backupDB = CreateDB(dbPath);
		backupDB->ContinueScan();
	}
	else
	{
		retValue = ExitCode_GeneralFailure;
	}

	logger->DebugFormat("Operation:[Scan] dbPath:[%s] retValue:[%d]", dbPath.c_str(), retValue);
	return retValue;
}

int BackupOperation::DiffCalc(boost::program_options::variables_map& vm)
{
	int retValue = ExitCode_Success;

	std::string dbPath = getValueAsString(vm, "BackupDB.path");

	logger->DebugFormat("Operation:[DiffCalc] dbPath:[%s]", dbPath.c_str());

	bool doesExists = boost::filesystem::exists(dbPath);
	if (doesExists)
	{
		auto backupDB = CreateDB(dbPath);
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

int BackupOperation::FileUpload(boost::program_options::variables_map& vm)
{
	int retValue = ExitCode_Success;

	std::string dbPath = getValueAsString(vm, "BackupDB.path");

	logger->DebugFormat("Operation:[FileUpload] dbPath:[%s]", dbPath.c_str());

	bool doesExists = boost::filesystem::exists(dbPath);
	if (doesExists)
	{
		auto backupDB = CreateDB(dbPath);
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

int BackupOperation::Complete(boost::program_options::variables_map& vm)
{
	int retValue = ExitCode_Success;

	std::string dbPath = getValueAsString(vm, "BackupDB.path");

	logger->DebugFormat("Operation:[Complete] dbPath:[%s]", dbPath.c_str());

	bool doesExists = boost::filesystem::exists(dbPath);
	if (doesExists)
	{
		auto backupDB = CreateDB(dbPath);
		backupDB->ContinueScan();
		backupDB->ContinueDiffCalc();
		std::shared_ptr<IFileRepositoryDB> fileRepDB = getFileRepository(vm);
		backupDB->ContinueUpload(fileRepDB);
		backupDB->Complete();

		std::string name = getValueAsString(vm, "name");
		auto RepoDB = getRepository(vm);
		auto backupdef = RepoDB->GetBackupDef(name);

		auto backupInfo = RepoDB->CreateBackupInfo(*backupdef);
		if (this->strategy != nullptr && this->strategy->successFunc != nullptr)
		{
			this->strategy->successFunc(backupInfo);
		}
		boost::filesystem::path backupDBPath = getValueAsString(vm, "BackupDB.path");
		RepoDB->CopyBackupDBStateIntoRepoAndComplete(backupInfo, backupDBPath);
	}
	else
	{
		retValue = ExitCode_GeneralFailure;
	}

	logger->DebugFormat("Operation:[Complete] dbPath:[%s] retValue:[%d]", dbPath.c_str(), retValue);
	return retValue;
}

int BackupOperation::All(boost::program_options::variables_map& vm)
{
	int retValue = ExitCode_Success;

	std::string name = getValueAsString(vm, "name");

	logger->DebugFormat("Operation:[Backup] Name:[%s] Step [All]", name.c_str());

	auto RepoDB = getRepository(vm);
	auto backupdef = RepoDB->GetBackupDef(name);
	if (backupdef != nullptr)
	{
		std::shared_ptr<IFileRepositoryDB> fileRepDB = getFileRepository(vm);
		boost::filesystem::path backupDBPath = getValueAsString(vm, "BackupDB.path");
		auto backupInfo = RepoDB->Backup(IRepositoryDB::BackupParameters().BackupDefId(backupdef->id), fileRepDB, backupDBPath);
		if (this->strategy != nullptr && this->strategy->successFunc != nullptr)
		{
			this->strategy->successFunc(backupInfo);
		}
	}

	logger->DebugFormat("Operation:[Backup] Name:[%s] Step [All] retValue:[%d]", name.c_str(), retValue);
	return retValue;
}

int BackupOperation::Operate(boost::program_options::variables_map& vm)
{
	int retValue = ExitCode_Success;
	std::string step = getValueAsString(vm, "bstep", "All");

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

std::shared_ptr<Operation> BackupFactory()
{
	return std::make_shared<BackupOperation>();
}

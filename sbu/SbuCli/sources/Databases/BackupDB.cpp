#include "BackupDB.h"
#include "utils.h"
#include "SbuDatabaseSQLite.h"

using namespace boost::filesystem;

static std::shared_ptr<ILogger> logger = LoggerFactory::getLogger("application.BackupDB");

class BackupDB : public IBackupDB
{
public:
	BackupDB(path dbPath) 
	{
		logger->DebugFormat("BackupDB::BackupDB() dbpath:[%s]", dbPath.string().c_str());
		bool exists = boost::filesystem::exists(dbPath);
		this->db = getOrCreateDb(dbPath, Text_Resource::BackupDB);
		if (exists) 
		{
			auto infoQuery = db->CreateStatement("SELECT RootPath FROM GeneralInfo");
			if (infoQuery->executeStep())
			{
				this->root = from_utf8(infoQuery->getColumn("RootPath")->getString());
				logger->DebugFormat("BackupDB::BackupDB() dbpath:[%s] already exists Using RootPath [%s]", dbPath.string().c_str(), this->root.c_str());
			}
		}
	}

	void UpdateAction(const std::string& action)
	{
		auto updateQuery = db->CreateStatement("UPDATE GeneralInfo SET LastAction=:action");
		updateQuery->bind(":action", action);
		updateQuery->exec();

		logger->DebugFormat("BackupDB::UpdateAction() Updating with action[%s]", action.c_str());
	}

	virtual void StartScan(path dir) override
	{
		root = dir;
		logger->DebugFormat("BackupDB::StartScan() dir:[%s] Start", dir.string().c_str());

		AddToExecutionLog(db, "Start Scan", to_utf8(dir.generic_wstring()));

		auto insertQuery = db->CreateStatement("INSERT INTO GeneralInfo (RootPath, Created, LastAction) VALUES (:root ,:created, :lastAction)");

		insertQuery->bind(":root", to_utf8(dir));
		insertQuery->bind(":created", return_current_time_and_date());
		insertQuery->bind(":lastAction", std::string("StartScan"));
		insertQuery->exec();

		this->AddDirectoryToScan(dir);

		logger->DebugFormat("BackupDB::StartScan() dir:[%s] End", dir.string().c_str());
	}

	virtual void ContinueScan() override
	{
		logger->DebugFormat("BackupDB::ContinueScan() Start");

		auto shouldRunStage = db->CreateStatement("SELECT FileUploadComplete FROM GeneralInfo WHERE ScanComplete IS NOT NULL");
		if (shouldRunStage->executeStep())
		{
			logger->DebugFormat("BackupDB::ContinueUpload() Already done");
			return;
		}

		this->UpdateAction("ContinueScan");
		AddToExecutionLog(db, "Continue Scan", "");
		auto query = db->CreateStatement("SELECT ID,Path FROM NextScan");
		while (query->executeStep())
		{
			Integer id = query->getColumn("ID")->getInt64();
			path currentPath = from_utf8(query->getColumn("Path")->getString());
			logger->DebugFormat("BackupDB::ContinueScan() id:[%d] currentPath[%s]", id, currentPath.string().c_str());
			HandleDirectory(id, currentPath);
			query->reset();
		}

		auto completeStage = db->CreateStatement("UPDATE GeneralInfo SET ScanComplete=:date");
		completeStage->bind(":date", return_current_time_and_date());
		completeStage->exec();

		AddToExecutionLog(db, "End of Scan", "");
		logger->DebugFormat("BackupDB::ContinueScan() End");
	}

	virtual bool IsScanDone() override
	{
		bool retValue = true;
		auto query = db->CreateStatement("SELECT ID,Path FROM NextScan");
		if (query->executeStep())
			retValue = false;

		logger->DebugFormat("BackupDB::IsScanDone() retValue:[%d]", retValue);
		return retValue;
	}

	virtual void StartDiffCalc() override
	{
		this->UpdateAction("StartDiffCalc");
		AddToExecutionLog(db, "Start DiffCalc", "");
		logger->Debug("Adding deleted files into NextState");
		try {
			auto markDeleted = db->CreateStatement(Text_Resource::MarkDeleted);
			markDeleted->exec();
		}

		catch (std::exception ex)
		{
			logger->ErrorFormat("BackupDB::StartDiffCalc() failed exception:[%s]", ex.what());
			throw;
		}
	}

	virtual void ContinueDiffCalc() override
	{
		logger->DebugFormat("BackupDB::ContinueDiffCalc()");
		auto shouldRunStage = db->CreateStatement("SELECT FileUploadComplete FROM GeneralInfo WHERE DiffComplete IS NOT NULL");
		if (shouldRunStage->executeStep())
		{
			logger->DebugFormat("BackupDB::ContinueUpload() Already done");
			return;
		}

		try {
			this->UpdateAction("ContinueDiffCalc");
			AddToExecutionLog(db, "ContinueDiffCalc", "");
			auto calcDigestQuery = db->CreateStatement(Text_Resource::findHashCalcQuery);
			while (calcDigestQuery->executeStep())
			{
				auto filePath = from_utf8(calcDigestQuery->getColumn("Path")->getString());

				auto fullPath = root / boost::filesystem::path(filePath);
				auto startDigest = std::chrono::system_clock::now();
				auto digest = calcHash(fullPath);
				auto endDigest = std::chrono::system_clock::now();

				auto updateDigest = db->CreateStatement("Update Entries SET StartDigestCalc=:startDigest, EndDigestCalc=:endDigest, DigestType=:digestType, DigestValue=:digestValue WHERE Path=:path");
				updateDigest->bind(":startDigest", get_string_from_time_point(startDigest));
				updateDigest->bind(":endDigest", get_string_from_time_point(endDigest));
				updateDigest->bind(":digestType", "SHA1");
				updateDigest->bind(":digestValue", digest);
				updateDigest->bind(":path", to_utf8(filePath));
				updateDigest->exec();

				logger->DebugFormat("BackupDB::ContinueDiffCalc() path:[%s] digestType:[SHA1] digestValue:[%s]",
					to_utf8(filePath).c_str(),
					digest.c_str());
			}

			auto addMissing = db->CreateStatement(Text_Resource::AddMissing);
			addMissing->exec();

			auto addUpdated = db->CreateStatement(Text_Resource::AddUpdated);
			addUpdated->exec();

			auto completeStage = db->CreateStatement("UPDATE GeneralInfo SET DiffComplete=:date");
			completeStage->bind(":date", return_current_time_and_date());
			completeStage->exec();

		}
		catch (std::exception ex)
		{
			logger->ErrorFormat("BackupDB::ContinueDiffCalc() Error with exception:[%s]", ex.what());
			throw;
		}
	}

	virtual bool IsDiffCalcDone() override
	{
		logger->InfoFormat("BackupDB::IsDiffCalcDone()");
		return false;
	}

	virtual void StartUpload(std::shared_ptr<IFileRepositoryDB> fileDB) override
	{
		logger->InfoFormat("BackupDB::StartUpload()");
		AddToExecutionLog(db, "StartUpload", "");
		this->UpdateAction("StartUpload");
	}

	virtual void ContinueUpload(std::shared_ptr<IFileRepositoryDB> fileDB) override
	{
		logger->InfoFormat("BackupDB::ContinueUpload()");
		auto shouldRunStage = db->CreateStatement("SELECT FileUploadComplete FROM GeneralInfo WHERE FileUploadComplete IS NOT NULL");
		if (shouldRunStage->executeStep())
		{
			logger->DebugFormat("BackupDB::ContinueUpload() Already done");
			return;
		}

		this->UpdateAction("ContinueUpload");
		AddToExecutionLog(db, "ContinueUpload", "");
		auto uploadQuery = db->CreateStatement(Text_Resource::findUploadQuery);
		while (uploadQuery->executeStep())
		{
			auto filePath = from_utf8(uploadQuery->getColumn("Path")->getString());
			auto digest = uploadQuery->getColumn("DigestValue")->getString();
			auto digestType = uploadQuery->getColumn("DigestType")->getString();
			auto fullPath = root / boost::filesystem::path(filePath);

			auto start = std::chrono::system_clock::now();
			auto fileHandle = fileDB->AddFile(fullPath, digestType, digest);
			auto end = std::chrono::system_clock::now();

			auto transaction = db->CreateTransaction();

			auto updateEntries = db->CreateStatement("UPDATE Entries SET StartUpload=:start, EndUpload=:end, FileHandle=:handle WHERE Path=:path");
			updateEntries->bind(":start", get_string_from_time_point(start));
			updateEntries->bind(":end", get_string_from_time_point(end));
			updateEntries->bind(":handle", fileHandle);
			updateEntries->bind(":path", to_utf8(filePath));
			updateEntries->exec();

			auto updateCurrent = db->CreateStatement("UPDATE NextState SET FileHandle=:handle, UploadState=:state WHERE Path=:path");
			updateCurrent->bind(":handle", fileHandle);
			updateCurrent->bind(":state", "Uploaded");
			updateCurrent->bind(":path", to_utf8(filePath));
			updateCurrent->exec();

			transaction->commit();
		}

		auto completeStage = db->CreateStatement("UPDATE GeneralInfo SET FileUploadComplete=:date");
		completeStage->bind(":date", return_current_time_and_date());
		completeStage->exec();

		fileDB->Complete();
	}

	virtual bool IsUploadDone() override
	{
		bool retValue = true;
		auto uploadQuery = db->CreateStatement(Text_Resource::findUploadQuery);
		if (uploadQuery->executeStep())
			retValue = false;

		logger->InfoFormat("BackupDB::IsUploadDone() retValue:[%d]", retValue);
		return retValue;
	}

	virtual void Complete() override
	{
		auto completeStage = db->CreateStatement("UPDATE GeneralInfo SET FinalizationComplete=:date");
		completeStage->bind(":date", return_current_time_and_date());
		completeStage->exec();

		this->UpdateAction("Complete");
		AddToExecutionLog(db, "Complete", "");
		logger->InfoFormat("BackupDB::Complete()");
	}

protected:
	void AddDirectoryToScan(path dir)
	{
		auto insertQuery = db->CreateStatement("INSERT INTO Scan (Path, Added) VALUES (:path,:added)");
		auto added = return_current_time_and_date();
		insertQuery->bind(":path", to_utf8(dir));
		insertQuery->bind(":added", added);
		auto result = insertQuery->exec();
		logger->DebugFormat("BackupDB::AddDirectoryToScan() dir:[%s] added:[%s] result:[%d]",
			dir.string().c_str(),
			added.c_str(),
			result);
	}

	void UpdatedStarted(Integer id)
	{
		auto updateStartedQuery =db->CreateStatement("UPDATE Scan SET Started=:started WHERE ID=:id");
		updateStartedQuery->bind(":started", return_current_time_and_date());
		updateStartedQuery->bind(":id", id);
		updateStartedQuery->exec();
		logger->DebugFormat("BackupDB::UpdatedStarted() id:[%d]", id);
	}

	void UpdatedCompleted(Integer id)
	{
		auto updateCompletedQuery =db->CreateStatement("UPDATE Scan SET Completed=:completed WHERE ID=:id");
		updateCompletedQuery->bind(":completed", return_current_time_and_date());
		updateCompletedQuery->bind(":id", id);
		updateCompletedQuery->exec();
		logger->DebugFormat("BackupDB::UpdatedCompleted() id:[%d]", id);
	}

	void InsertDirectoryToEntries(path dir)
	{
		static std::string insertQuerySQL = Text_Resource::InsertDirectory;
		logger->DebugFormat("BackupDB::InsertDirectoryToEntries() dir:[%ws]", dir.string().c_str());
		sbu_stats::Stat result;
		auto stat_Result = sbu_stats::stat(dir, &result);
		try {
			auto insertQuery =db->CreateStatement(insertQuerySQL);
			insertQuery->bind(":path", to_utf8(relative(dir, root)));
			insertQuery->bind(":type", "Directory");
			insertQuery->bind(":added", return_current_time_and_date());
			insertQuery->bind(":created", return_time_and_date(result.st_ctime));
			insertQuery->bind(":modified", return_time_and_date(result.st_mtime));
			insertQuery->bind(":accessed", return_time_and_date(result.st_atime));
			insertQuery->exec();
		}
		catch (std::exception ex)
		{
			logger->ErrorFormat("BackupDB::InsertDirectoryToEntries() Failed to insert dir:[%s] exception:[%s]",
				dir.string().c_str(), ex.what());
			AddToExecutionLog(db, "Error In Handling Directory", to_utf8(dir.generic_wstring()));
		}
	}

	void ScanDirectory(path dir)
	{
		logger->DebugFormat("BackupDB::ScanDirectory() path:[%s]", dir.string().c_str());
		try {
			directory_iterator it{ dir };
			while (it != directory_iterator())
			{
				auto currentEntry = *it;
				auto currentType = currentEntry.status().type();
				switch (currentType)
				{
				case file_type::directory_file:
					AddDirectoryToScan(currentEntry.path());
					break;

				case file_type::regular_file:
					HandleFile(currentEntry.path());
					break;
				default:
					break;
				}

				it++;
			}
		}
		catch (std::exception ex)
		{
			logger->ErrorFormat("BackupDB::ScanDirectory() Handling dir:[%s] exception:[%s]",
				dir.string().c_str(), ex.what());

			AddToExecutionLog(db, "Error In Scanning Directory", to_utf8(dir.generic_wstring()));
		}
	}

	void HandleDirectory(Integer id, path dir)
	{
		logger->DebugFormat("BackupDB::HandleDirectory() Id:[%ld] Path:[%s]", id, dir.string().c_str());
		auto transaction = db->CreateTransaction();

		UpdatedStarted(id);
		InsertDirectoryToEntries(dir);
		ScanDirectory(dir);
		UpdatedCompleted(id);

		transaction->commit();
	}

	void HandleFile(path file)
	{
		static std::string insertQuerySQL = Text_Resource::InsertFile;
		sbu_stats::Stat result;
		auto statResult = sbu_stats::stat(file, &result);
		if (statResult == 0) 
		{
			logger->DebugFormat("BackupDB::HandleFile() file:[%s] statResult:[%d]", file.string().c_str(), statResult);
			try {
				auto insertQuery = db->CreateStatement(insertQuerySQL);
				insertQuery->bind(":path", to_utf8(relative(file, root)));
				insertQuery->bind(":type", "File");
				insertQuery->bind(":added", return_current_time_and_date());
				insertQuery->bind(":size", result.st_size);
				insertQuery->bind(":created", return_time_and_date(result.st_ctime));
				insertQuery->bind(":modified", return_time_and_date(result.st_mtime));
				insertQuery->bind(":accessed", return_time_and_date(result.st_atime));
				insertQuery->exec();
			}
			catch (std::exception ex)
			{
				logger->ErrorFormat("BackupDB::ScanDirectory() Handling file:[%s] exception:[%s]",
					file.string().c_str(), ex.what());

				AddToExecutionLog(db, "Error In Handling File", to_utf8(file.generic_wstring()));
			}
		}
		else
		{
			logger->ErrorFormat("BackupDB::ScanDirectory() Handling file:[%s] StatResult:[%d]",
				file.string().c_str(), statResult);

			AddToExecutionLog(db, "Error In Handling File statResult", to_utf8(file.generic_wstring()));
		}

	}
private:
	std::shared_ptr<ISbuDBDatabase> db;
	path root;
};

std::shared_ptr<IBackupDB> CreateDB(boost::filesystem::path dbPath)
{
	logger->DebugFormat("Creating BackupDB dbPath:[%s]", dbPath.string().c_str());
	return std::make_shared<BackupDB>(dbPath);
}

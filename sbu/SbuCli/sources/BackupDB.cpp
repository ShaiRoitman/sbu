#include "BackupDB.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "utils.h"

using namespace boost::filesystem;
using namespace SQLite;

static auto logger = LoggerFactory::getLogger("application.BackupDB");

class BackupDB : public IBackupDB
{
public:
	BackupDB(path dbPath)
	{
		this->db = getOrCreateDb(dbPath, Text_Resource::BackupDB);
	}

	virtual void ContinueScan() override
	{
		AddToExecutionLog("Continue Scan", "");
		SQLite::Statement query(*db, "SELECT ID,Path from NextScan");
		while (query.executeStep())
		{
			Integer id = query.getColumn("ID").getInt64();
			path currentPath = from_utf8(query.getColumn("Path").getString());
			HandleDirectory(id, currentPath);
			query.reset();
		}

		AddToExecutionLog("End of Scan", "");
	}

	virtual void StartScan(path dir) override
	{
		root = dir;

		AddToExecutionLog("Start Scan", to_utf8(dir.generic_wstring()));

		SQLite::Statement insertQuery(*db, "INSERT INTO GeneralInfo (RootPath, Created) Values (:root ,:created)");

		insertQuery.bind(":root", to_utf8(dir));
		insertQuery.bind(":created", return_current_time_and_date());
		insertQuery.exec();

		this->AddDirectoryToScan(dir);
		ContinueScan();
	}

	virtual bool IsScanDone() override
	{
		SQLite::Statement query(*db, "SELECT ID,Path from NextScan");
		if (query.executeStep())
			return false;
		else
			return true;
	}

protected:
	void AddToExecutionLog(const std::string& comment, const std::string& argument)
	{
		SQLite::Statement insertQuery(*db, "INSERT INTO ExecutionLog (EventTime, Comment, Argument) Values (:time, :comment, :arg)");

		insertQuery.bind(":time", return_current_time_and_date());
		insertQuery.bind(":comment", comment);
		insertQuery.bind(":arg", argument);
		insertQuery.exec();
	}

	void AddDirectoryToScan(path dir)
	{
		SQLite::Statement insertQuery(*db, "INSERT INTO Scan (Path, Added) Values (:path,:added)");

		insertQuery.bind(":path", to_utf8(dir));
		insertQuery.bind(":added", return_current_time_and_date());
		insertQuery.exec();
	}

	void UpdatedStarted(Integer id)
	{
		SQLite::Statement updateStartedQuery(*db, "Update Scan Set Started=:started WHERE ID=:id");
		updateStartedQuery.bind(":started", return_current_time_and_date());
		updateStartedQuery.bind(":id", id);
		updateStartedQuery.exec();
	}

	void UpdatedCompleted(Integer id)
	{
		SQLite::Statement updateCompletedQuery(*db, "Update Scan Set Completed=:completed WHERE ID=:id");
		updateCompletedQuery.bind(":completed", return_current_time_and_date());
		updateCompletedQuery.bind(":id", id);
		updateCompletedQuery.exec();
	}

	void InsertDirectoryToEntries(path dir)
	{
		static std::string insertQuerySQL = Text_Resource::InsertDirectory;
		struct _stat64 result;
		_wstati64(dir.generic_wstring().c_str(), &result);
		try {
			SQLite::Statement insertQuery(*db, insertQuerySQL);
			insertQuery.bind(":path", to_utf8(relative(dir, root)));
			insertQuery.bind(":type", "Directory");
			insertQuery.bind(":added", return_current_time_and_date());
			insertQuery.bind(":created", return_time_and_date(result.st_ctime));
			insertQuery.bind(":modified", return_time_and_date(result.st_mtime));
			insertQuery.bind(":accessed", return_time_and_date(result.st_atime));
			insertQuery.exec();
		}
		catch (std::runtime_error ex)
		{
			AddToExecutionLog("Error In Handling Directory", to_utf8(dir.generic_wstring()));
		}
	}

	void ScanDirectory(path dir)
	{
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
		catch (std::runtime_error ex)
		{
			AddToExecutionLog("Error In Scanning Directory", to_utf8(dir.generic_wstring()));
		}
	}

	void HandleDirectory(Integer id, path dir)
	{
		Transaction transaction(*db);

		UpdatedStarted(id);
		InsertDirectoryToEntries(dir);
		ScanDirectory(dir);
		UpdatedCompleted(id);

		transaction.commit();
	}

	void HandleFile(path file)
	{
		static std::string insertQuerySQL = Text_Resource::InsertFile;
		struct _stat64 result;
		_wstati64(file.generic_wstring().c_str(), &result);
		try {
			SQLite::Statement insertQuery(*db, insertQuerySQL);
			insertQuery.bind(":path", to_utf8(relative(file, root)));
			insertQuery.bind(":type", "File");
			insertQuery.bind(":added", return_current_time_and_date());
			insertQuery.bind(":size", result.st_size);
			insertQuery.bind(":created", return_time_and_date(result.st_ctime));
			insertQuery.bind(":modified", return_time_and_date(result.st_mtime));
			insertQuery.bind(":accessed", return_time_and_date(result.st_atime));
			insertQuery.exec();
		}
		catch (std::runtime_error ex)
		{
			AddToExecutionLog("Error In Handling File", to_utf8(file.generic_wstring()));
		}
	}

	virtual void StartDiffCalc() override
	{
		try {
			Transaction transaction(*db);

			logger->Debug("Marking files as Deleted existing in the backup but not in the scan");
			SQLite::Statement markDeleted(*db, Text_Resource::MarkDeleted);
			markDeleted.exec();

			logger->Debug("Marking files as Added existing in the scan but not in the backup");
			SQLite::Statement addMissingQuery(*db, Text_Resource::AddMissing);
			addMissingQuery.exec();

			SQLite::Statement markUnchangedQuery(*db, Text_Resource::MarkUnchanged);
			markUnchangedQuery.exec();

			SQLite::Statement markUpdated(*db, Text_Resource::MarkUpdated);
			markUpdated.exec();

			transaction.commit();
		}

		catch (std::runtime_error ex)
		{
			int k = 3;
		}

	}

	virtual void ContinueDiffCalc() override
	{
		SQLite::Statement calcDigestQuery(*db, Text_Resource::findHashCalcQuery);
		while (calcDigestQuery.executeStep())
		{
			auto id = calcDigestQuery.getColumn("ID").getInt();
			auto filePath = from_utf8(calcDigestQuery.getColumn("Path").getString());
			auto status = calcDigestQuery.getColumn("Status").getString();

			auto fullPath = root / boost::filesystem::path(filePath);
			auto startDigest = std::chrono::system_clock::now();
			auto digest = calcHash(fullPath);
			auto endDigest = std::chrono::system_clock::now();

			SQLite::Statement updateDigest(*db, "Update Entries SET StartDigestCalc=:startDigest, EndDigestCalc=:endDigest, DigestType=:digestType, DigestValue=:digestValue WHERE Path=:path");
			updateDigest.bind(":startDigest", get_string_from_time_point(startDigest));
			updateDigest.bind(":endDigest", get_string_from_time_point(endDigest));
			updateDigest.bind(":digestType", "SHA1");
			updateDigest.bind(":digestValue", digest);
			updateDigest.bind(":path", to_utf8(filePath));
			updateDigest.exec();

			if (status == "Added")
			{
				SQLite::Statement updateCurrentDigest(*db, "Update CurrentState SET DigestType=:digestType, DigestValue=:digestValue WHERE ID=:id");
				updateCurrentDigest.bind(":digestType", "SHA1");
				updateCurrentDigest.bind(":digestValue", digest);
				updateCurrentDigest.bind(":id", id);
				updateCurrentDigest.exec();
			}
		}

		SQLite::Statement markUpdatedQuery(*db, Text_Resource::MarkUpdated);
		markUpdatedQuery.exec();

		SQLite::Statement markUnchangedQuery(*db, Text_Resource::MarkUnchanged);
		markUnchangedQuery.exec();
	}

	virtual bool IsDiffCalcDone() override
	{
		return false;
	}

	virtual void StartUpload(std::shared_ptr<IFileRepositoryDB> fileDB) override
	{
	}

	virtual void ContinueUpload(std::shared_ptr<IFileRepositoryDB> fileDB) override
	{
		SQLite::Statement uploadQuery(*db, Text_Resource::findUploadQuery);
		while (uploadQuery.executeStep())
		{
			auto id = uploadQuery.getColumn("ID").getInt();
			auto filePath = from_utf8(uploadQuery.getColumn("Path").getString());
			auto digest = uploadQuery.getColumn("DigestValue").getString();
			auto digestType = uploadQuery.getColumn("DigestType").getString();
			auto fullPath = root / boost::filesystem::path(filePath);

			auto start = std::chrono::system_clock::now();
			auto fileHandle = fileDB->AddFile(fullPath, digestType, digest);
			auto end = std::chrono::system_clock::now();

			SQLite::Statement updateEntries(*db, "Update Entries SET StartUpload=:start, EndUpload=:end, FileHandle=:handle WHERE Path=:path");
			updateEntries.bind(":start", get_string_from_time_point(start));
			updateEntries.bind(":end", get_string_from_time_point(end));
			updateEntries.bind(":handle", fileHandle);
			updateEntries.bind(":path", to_utf8(filePath));
			updateEntries.exec();

			SQLite::Statement updateCurrent(*db, "Update CurrentState SET FileHandle=:handle, UploadState=:state WHERE ID=:id");
			updateCurrent.bind(":handle", fileHandle);
			updateCurrent.bind(":state", "Uploaded");
			updateCurrent.bind(":id", id);
			updateCurrent.exec();
		}
	}

	virtual bool IsUploadDone() override
	{
		SQLite::Statement uploadQuery(*db, Text_Resource::findUploadQuery);
		if (uploadQuery.executeStep())
			return false;
		else
			return true;
	}

private:
	std::shared_ptr<SQLite::Database> db;
	path root;
};

std::shared_ptr<IBackupDB> CreateSQLiteDB(boost::filesystem::path dbPath)
{
	logger->DebugFormat("Creating BackupDB dbPath:[%s]", dbPath.string().c_str());
	return std::make_shared<BackupDB>(dbPath);
}
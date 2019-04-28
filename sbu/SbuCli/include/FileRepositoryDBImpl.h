#include "FileRepositoryDB.h"
#include "Loggers.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "Poco/Types.h"
#include "Poco/Zip/ZipManipulator.h"
#include <map>

class ZipWrapper
{
public:
	ZipWrapper(const std::string& fileName);
	virtual ~ZipWrapper();
	bool ExtractFile(const std::string& handle, const std::string& path);
	bool Close();

	std::shared_ptr<Poco::Zip::ZipArchive> zipArchive;
	std::shared_ptr <std::ifstream> zipArchiveStream;
	std::string zipArchiveStreamName;
	std::shared_ptr<ILogger> logger;
};

class MultiFile
{
public:
	MultiFile();
	virtual ~MultiFile();
	Poco::UInt64 GetSize();
	bool AddFile(boost::filesystem::path file, const std::string& digest);
	bool HasFile(const std::string& digest);
	bool Close();
public:
	struct fileEntry
	{
		boost::filesystem::path file;
		std::string digest;
		long long size;
	};

	Poco::UInt64 totalSize;
	Poco::UInt64 fileSize;

	std::string zipFile;
	std::map<std::string, fileEntry> entries;
	std::shared_ptr <Poco::Zip::ZipManipulator> zip;
	std::shared_ptr<ILogger> logger;
};

class FileSystemStorageHandler : public IStorageHandler
{
public:
	FileSystemStorageHandler(boost::filesystem::path dataRootPath);
	virtual bool CopyFileToRepository(const IFileRepositoryDB::RepoHandle& handle, boost::filesystem::path srcFilePath) override;
	virtual bool CopyFileFromRepository(const IFileRepositoryDB::RepoHandle& handle, boost::filesystem::path dstFilePath) override;

public:
	boost::filesystem::path dataRootPath;
};

class FileRepositoryDB : public IFileRepositoryDB
{
public:
	FileRepositoryDB(std::shared_ptr<FileSystemStorageHandler> fileHandler, boost::filesystem::path dbPath, long long smallFileThreshold, long long bulkSize);
	virtual RepoHandle AddFile(boost::filesystem::path file, const std::string& digestType, const std::string& digest) final;
	virtual bool HasFile(const RepoHandle& handle) final;
	virtual bool GetFile(const RepoHandle& handle, boost::filesystem::path outFilePath) final;
	virtual void Complete() final;

protected:
	std::shared_ptr<FileSystemStorageHandler> fileHandler;
private:
	bool GetFileLocalPath(const RepoHandle& handle, boost::filesystem::path* path);
	void SendMultiFile();
	std::shared_ptr<SQLite::Database> db;
	boost::filesystem::path dataRootPath;

	long long smallFileBulkThreshold;
	long long bulkSize;
	
	MultiFile multiFile;
	std::map<std::string, std::shared_ptr<ZipWrapper>> zipFiles;
};


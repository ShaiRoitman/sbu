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

class FileRepositoryDB : public IFileRepositoryDB
{
public:
	FileRepositoryDB(boost::filesystem::path dbPath, boost::filesystem::path dataRootPath, long long smallFileThreshold, long long bulkSize);
	virtual RepoHandle AddFile(boost::filesystem::path file, const std::string& digestType, const std::string& digest) final;
	virtual bool HasFile(const RepoHandle& handle) final;
	virtual bool GetFile(const RepoHandle& handle, boost::filesystem::path outFilePath) final;
	virtual void Complete() final;
protected:
	virtual bool CopyFileToRepository(const RepoHandle& handle, boost::filesystem::path filePath) ;
	virtual bool CopyFileFromRepository(const RepoHandle& handle, boost::filesystem::path filePath) ;
private:
	bool GetFileLocalPath(const RepoHandle& handle, boost::filesystem::path* path);
	void SendMultiFile();
	std::shared_ptr<SQLite::Database> db;
	std::shared_ptr<ILogger> logger;
	boost::filesystem::path dataRootPath;

	long long smallFileBulkThreshold;
	long long bulkSize;

	MultiFile multiFile;
	std::map<std::string, std::shared_ptr<ZipWrapper>> zipFiles;
};


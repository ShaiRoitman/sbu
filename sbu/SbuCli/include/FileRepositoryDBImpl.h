#include "FileRepositoryDB.h"
#include "Loggers.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "Poco/Types.h"
#include "Poco/Zip/ZipManipulator.h"
#include <map>

class MultiFile
{
public:
	MultiFile();
	MultiFile(const std::string& fileName);
	virtual ~MultiFile();
	Poco::UInt64 GetSize();
	bool AddFile(boost::filesystem::path file, const std::string& digest);
	bool HasFile(const std::string& digest);
	bool ExtractFile(const std::string& handle, const std::string& path);
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
	Poco::Zip::ZipManipulator* zip;
	Poco::Zip::ZipArchive* zipArchive;
	std::ifstream* zipArchiveStream;

	std::shared_ptr<ILogger> logger;
};

class FileRepositoryDB : public IFileRepositoryDB
{
public:
	FileRepositoryDB(boost::filesystem::path dbPath, boost::filesystem::path dataRootPath, long long smallFileThreshold, long long bulkSize);
	virtual bool HasFile(const std::string& handle) override;
	virtual bool HasFile(const std::string& handle, boost::filesystem::path* path);
	virtual std::string AddFile(boost::filesystem::path file, const std::string& digestType, const std::string& digest) override;
	virtual bool GetFile(const std::string& handle, boost::filesystem::path outFilePath) override;
	virtual void Complete() override;
protected:
	void SendMultiFile();
private:
	std::shared_ptr<SQLite::Database> db;
	std::shared_ptr<ILogger> logger;
	boost::filesystem::path dataRootPath;
	
	long long smallFileBulkThreshold;
	long long bulkSize;

	MultiFile multiFile;
	std::map<std::string, MultiFile*> zipFiles;
};

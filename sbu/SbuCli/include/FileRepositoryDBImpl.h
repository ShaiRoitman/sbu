#include "FileRepositoryDB.h"
#include "Loggers.h"
#include "SQLiteCpp/SQLiteCpp.h"

class FileRepositoryDB : public IFileRepositoryDB
{
public:
	FileRepositoryDB(boost::filesystem::path dbPath, boost::filesystem::path dataRootPath, long long smallFileThreshold, long long bulkSize);
	virtual bool HasFile(const std::string& handle) override;
	virtual bool HasFile(const std::string& handle, boost::filesystem::path* path);
	virtual std::string AddFile(boost::filesystem::path file, const std::string& digestType, const std::string& digest) override;
	virtual bool GetFile(const std::string& handle, boost::filesystem::path outFilePath) override;
	virtual void Complete() override;
private:
	std::shared_ptr<SQLite::Database> db;
	std::shared_ptr<ILogger> logger;
	boost::filesystem::path dataRootPath;
	
	long long smallFileBulkThreshold;
	long long bulkSize;
};

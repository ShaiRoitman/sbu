#include "FileRepositoryDB.h"
#include "Loggers.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "MultiFile.h"
#include "ZipWrapper.h"
#include <map>

class FileRepositoryDB : public IFileRepositoryDB
{
public:
	FileRepositoryDB(std::shared_ptr<IStorageHandler> fileHandler, boost::filesystem::path dbPath, long long smallFileThreshold, long long bulkSize);
	virtual RepoHandle AddFile(boost::filesystem::path file, const std::string& digestType, const std::string& digest) final;
	virtual bool HasFile(const RepoHandle& handle) final;
	virtual bool GetFile(const RepoHandle& handle, boost::filesystem::path outFilePath) final;
	virtual void Complete() final;

protected:
	std::shared_ptr<IStorageHandler> fileHandler;
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


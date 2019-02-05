#include "FileRepositoryDB.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "utils.h"

using namespace boost::filesystem;
using namespace SQLite;

class FileRepositoryDB : public IFileRepositoryDB
{
public:
	FileRepositoryDB(boost::filesystem::path dbPath, boost::filesystem::path dataRootPath, bool create)
	{
		this->dbPath = dbPath;
		this->dataRootPath = dataRootPath;

		db = std::make_shared<SQLite::Database>(dbPath.string(), SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);

		if (create)
		{
			static std::string createQuery = Text_Resource::FileRepository;
			db->exec(createQuery);
		}

	}

	virtual std::string AddFile(boost::filesystem::path file, const std::string& digestType, const std::string& digest) override
	{
		std::string key = digest;
		SQLite::Statement query(*db, "SELECT Path FROM Files WHERE DigestValue=:key");
		query.bind(":key", key);
		if (!query.executeStep())
		{
			SQLite::Statement insertQuery(*db, "INSERT INTO Files (Path, Added, DigestType, DigestValue) VALUES (:path,:added,:digestType,:digestValue)");
			insertQuery.bind(":path", key);
			insertQuery.bind(":added", return_current_time_and_date());
			insertQuery.bind(":digestType", digestType);
			insertQuery.bind(":digestValue", key);
			insertQuery.exec();

//			path destPath = this->dataRootPath / file.relative_path();
			path destPath = this->dataRootPath / boost::filesystem::path(digest);
			boost::filesystem::create_directories(destPath.branch_path());
			try {
				copy_file(file, destPath, copy_option::overwrite_if_exists);
			}
			catch (std::exception ex)
			{
				int k = 3;
			}
		}

		return key;
	}

	virtual bool GetFile(const std::string& handle, boost::filesystem::path outFilePath) override
	{
		SQLite::Statement query(*db, "SELECT Path FROM Files WHERE DigestValue=:key");
		query.bind(":key", handle);
		bool retValue = false;
		if (query.executeStep())
		{
			path srcPath = this->dataRootPath / from_utf8(query.getColumn("Path").getString());
			boost::filesystem::create_directories(outFilePath.branch_path());
			copy_file(srcPath, outFilePath, copy_option::overwrite_if_exists);
			retValue = true;
		}
		
		return retValue;
	}
private:
	std::shared_ptr<SQLite::Database> db;
	path dataRootPath;
	path dbPath;
};

std::shared_ptr<IFileRepositoryDB> CreateFileRepositorySQLiteDB(boost::filesystem::path dbPath, boost::filesystem::path dataRootPath, bool create)
{
	return std::make_shared<FileRepositoryDB>(dbPath, dataRootPath, create);
}
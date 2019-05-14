#include "FileRepositoryDB.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "utils.h"

#include "FileRepositoryDBImpl.h"

#include "Poco/TemporaryFile.h"

using namespace boost::filesystem;
using namespace SQLite;


static std::shared_ptr<ILogger> logger = LoggerFactory::getLogger("application.FileRepositoryDB");

FileRepositoryDB::FileRepositoryDB(std::shared_ptr<IStorageHandler> fileHandler, boost::filesystem::path dbPath, long long smallFileThreshold, long long bulkSize)
{
	this->fileHandler = fileHandler;
	this->db = getOrCreateDb(dbPath, Text_Resource::FileRepository);
	this->smallFileBulkThreshold = smallFileThreshold;
	this->bulkSize = bulkSize;
	logger->DebugFormat("FileRepositoryDB::FileRepositoryDB() path:[%s] ", to_utf8(dbPath).c_str());

}
IFileRepositoryDB::RepoHandle FileRepositoryDB::AddFile(boost::filesystem::path file, const std::string& digestType, const std::string& digest)
{
	logger->DebugFormat("FileRepositoryDB::AddFile() path:[%s] digestType:[%s] digest:[%s]", file.string().c_str(), digestType.c_str(), digest.c_str());

	std::string key = digest;
	if (!this->HasFile(key))
	{
		if (!this->multiFile.HasFile(key))
		{
			auto fileSize = (long long)boost::filesystem::file_size(file);
			if (fileSize < this->smallFileBulkThreshold)
			{
				logger->DebugFormat("FileRepositoryDB::AddFile() small file size:[%ld] digest:[%s]", fileSize, digest.c_str());
				this->multiFile.AddFile(file, digest);
				if (this->multiFile.GetSize() > this->bulkSize)
				{
					this->SendMultiFile();
				}
			}
			else
			{
				logger->DebugFormat("FileRepositoryDB::AddFile() key [%s] is missing -> adding", key.c_str());
				if (this->fileHandler->CopyFileToRepository(digest, file) == false)
				{
					logger->ErrorFormat("FileRepositoryDB::AddFile() key [%s] Failed", key.c_str());
				}

				SQLite::Statement insertQuery(*db, "INSERT INTO Files (Path, Size, Added, DigestType, DigestValue) VALUES (:path,:size,:added,:digestType,:digestValue)");
				insertQuery.bind(":path", to_utf8(key));
				insertQuery.bind(":size", fileSize);
				insertQuery.bind(":added", return_current_time_and_date());
				insertQuery.bind(":digestType", digestType);
				insertQuery.bind(":digestValue", key);
				insertQuery.exec();
			}
		}
		else
		{
			logger->DebugFormat("FileRepositoryDB::AddFile() key:[%s] exists in multiFile", key.c_str());

		}
	}
	else
	{
		logger->DebugFormat("FileRepositoryDB::AddFile() key:[%s] exists", key.c_str());
	}

	return key;
}
bool FileRepositoryDB::HasFile(const RepoHandle& handle)
{
	return this->GetFileLocalPath(handle, nullptr);
}
bool FileRepositoryDB::GetFile(const RepoHandle& handle, boost::filesystem::path outFilePath)
{
	logger->DebugFormat("FileRepositoryDB::GetFile() handle:[%s] destination:[%s]",
		handle.c_str(),
		outFilePath.string().c_str());

	std::string key = handle;
	boost::filesystem::path srcPath;

	bool retValue = false;
	if (this->GetFileLocalPath(key, &srcPath))
	{
		boost::filesystem::create_directories(outFilePath.branch_path());
		if (copy_file_logged(srcPath, outFilePath) == false)
		{
			logger->WarningFormat("FileRepositoryDB::GetFile() handle:[%s] destination:[%s] Failed",
				handle.c_str(),
				outFilePath.string().c_str());
		}
	}
	else
	{
		logger->WarningFormat("FileRepositoryDB::GetFile() handle:[%s] destination:[%s] Failed Missing in repository",
			handle.c_str(),
			outFilePath.string().c_str());
	}

	return retValue;
}
void FileRepositoryDB::Complete()
{
	logger->DebugFormat("FileRepositoryDB::Complete()");
	if (this->multiFile.GetSize() != 0)
	{
		SendMultiFile();
	}
}
bool FileRepositoryDB::GetFileLocalPath(const RepoHandle& handle, boost::filesystem::path* path)
{
	std::string key = handle;
	SQLite::Statement query(*db, "SELECT Path,HostDigest FROM Files WHERE DigestValue=:key");
	query.bind(":key", key);
	bool retValue = false;
	if (query.executeStep())
	{
		if (path != nullptr)
		{
			auto pathColumn = query.getColumn("Path");
			if (pathColumn.isNull() == false)
			{
				auto tempFileName = Poco::TemporaryFile::tempName();
				this->fileHandler->CopyFileFromRepository(pathColumn.getString(), tempFileName);
				*path = tempFileName;
			}
			else
			{
				auto hostDigest = query.getColumn("HostDigest").getString();
				if (this->zipFiles.find(hostDigest) == this->zipFiles.end())
				{
					auto ziptempFileName = Poco::TemporaryFile::tempName();
					this->fileHandler->CopyFileFromRepository(hostDigest, ziptempFileName);
					this->zipFiles[hostDigest] = std::make_shared<ZipWrapper>(ziptempFileName);
				}
				auto tempFileName = Poco::TemporaryFile::tempName();
				auto multiFile = this->zipFiles[hostDigest];
				multiFile->ExtractFile(key, tempFileName);
				*path = tempFileName;
			}
		}
		retValue = true;
	}

	return retValue;
}
void FileRepositoryDB::SendMultiFile()
{
	this->multiFile.Close();
	Transaction transaction(*db);
	logger->DebugFormat("FileRepositoryDB::SendMultiFile()");

	auto digest = calcHash(this->multiFile.zipFile);

	for (std::map<std::string, MultiFile::fileEntry>::iterator iter = this->multiFile.entries.begin();
		iter != this->multiFile.entries.end();
		++iter)
	{
		auto currEntry = (*iter).second;
		
		SQLite::Statement insertQuery(*db, "INSERT INTO Files (Size, Added, DigestType, DigestValue, HostDigest) VALUES (:size,:added,:digestType,:digestValue,:hostDigest)");
		insertQuery.bind(":size", currEntry.size);
		insertQuery.bind(":added", return_current_time_and_date());
		insertQuery.bind(":digestType", "SHA1");
		insertQuery.bind(":digestValue", currEntry.digest);
		insertQuery.bind(":hostDigest", digest);
		insertQuery.exec();
	}


	auto fileSize = (long long)boost::filesystem::file_size(this->multiFile.zipFile);
	auto key = digest;

	if (this->fileHandler->CopyFileToRepository(digest, this->multiFile.zipFile) == false)
	{
		logger->ErrorFormat("FileRepositoryDB::AddFile() key:[%s] Failed", key.c_str());
	}

	SQLite::Statement insertQuery(*db, "INSERT INTO Files (Path, Size, Added, DigestType, DigestValue) VALUES (:path,:size,:added,:digestType,:digestValue)");
	insertQuery.bind(":path", to_utf8(digest));
	insertQuery.bind(":size", fileSize);
	insertQuery.bind(":added", return_current_time_and_date());
	insertQuery.bind(":digestType", "SHA1");
	insertQuery.bind(":digestValue", digest);
	insertQuery.exec();
	transaction.commit();

	this->multiFile = MultiFile();
}

std::shared_ptr<IFileRepositoryDB> CreateFileRepositorySQLiteDB(
	std::shared_ptr<IStorageHandler> storageHander,
	boost::filesystem::path dbPath,
	long minSizeToBulk,
	long bulkSize)
{
	logger->DebugFormat("Creating FileRepositoryDB dbPath:[%s] minFileToBulk:[%lld] fileBulkSize:[%lld]", 
		dbPath.string().c_str(), 
		minSizeToBulk,
		bulkSize);

	return std::make_shared<FileRepositoryDB>(
		storageHander,
		dbPath,
		minSizeToBulk, 
		bulkSize);
}


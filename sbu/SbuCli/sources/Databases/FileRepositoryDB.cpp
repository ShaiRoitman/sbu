#include "FileRepositoryDB.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "utils.h"

#include "FileRepositoryDBImpl.h"

#include "Poco/Zip/Compress.h"
#include "Poco/Zip/ZipManipulator.h"

#include <iostream>

using namespace boost::filesystem;
using namespace SQLite;

MultiFile::MultiFile() 
{
	this->totalSize = 0;
	this->fileSize = 0;
	this->zipFile = "c:\\workdir\\test.zip";
	boost::filesystem::remove(this->zipFile);
	zip = nullptr;
}

MultiFile::~MultiFile()
{
	this->Close();
}

Poco::UInt64 MultiFile::GetSize()
{
	return this->totalSize;
}

bool MultiFile::Close() 
{
	if (this->zip!= nullptr)
	{
		zip->commit();
		delete zip;
		zip = nullptr;
		return true;
	}

	return false;
}

bool MultiFile::AddFile(boost::filesystem::path file, const std::string& digest)
{
	if (this->HasFile(digest))
		return false;
	fileEntry newEntry;
	newEntry.digest = digest;
	newEntry.file = file;
	newEntry.size = (long long)boost::filesystem::file_size(file);
	entries[digest] = newEntry;
	if (boost::filesystem::exists(zipFile) == false && zip == nullptr)
	{
		std::ofstream out(zipFile, std::ios::binary);
		Poco::Zip::Compress c(out, true);
		c.close();
		if (zip != nullptr)
		{
			delete zip;
			zip = nullptr;
		}
		zip = new Poco::Zip::ZipManipulator(this->zipFile, false);
	}
	zip->addFile(digest, file.string());
	this->totalSize += newEntry.size;

	return true;
}

bool MultiFile::HasFile(const std::string& digest)
{
	if (entries.find(digest) != entries.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

FileRepositoryDB::FileRepositoryDB(boost::filesystem::path dbPath, boost::filesystem::path dataRootPath, long long smallFileThreshold, long long bulkSize) :
	logger(LoggerFactory::getLogger("application.FileRepositoryDB"))
{
	this->dataRootPath = dataRootPath;
	this->db = getOrCreateDb(dbPath, Text_Resource::FileRepository);
	this->smallFileBulkThreshold = smallFileThreshold;
	this->bulkSize = bulkSize;
	logger->DebugFormat("FileRepositoryDB::FileRepositoryDB() path:[%s] root:[%s]", to_utf8(dbPath).c_str(), to_utf8(dataRootPath).c_str());
}

bool FileRepositoryDB::HasFile(const std::string& handle)
{
	return this->HasFile(handle, nullptr);
}

bool FileRepositoryDB::HasFile(const std::string& handle, boost::filesystem::path* path)
{
	std::string key = handle;
	SQLite::Statement query(*db, "SELECT Path FROM Files WHERE DigestValue=:key");
	query.bind(":key", key);
	bool retValue = false;
	if (query.executeStep())
	{
		if (path != nullptr)
		{
			*path = this->dataRootPath / from_utf8(query.getColumn("Path").getString());
		}
		retValue = true;
	}

	return retValue;
}

std::string FileRepositoryDB::AddFile(boost::filesystem::path file, const std::string& digestType, const std::string& digest)
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
				this->multiFile.AddFile(file, digest);
				if (this->multiFile.GetSize() > this->bulkSize)
				{
					this->SendMultiFile();
				}
			}
			else
			{
				logger->DebugFormat("FileRepositoryDB::AddFile() key [%s] is missing -> adding", key.c_str());
				path destPath = this->dataRootPath / boost::filesystem::path(digest);
				boost::filesystem::create_directories(destPath.branch_path());
				if (copy_file_logged(file, destPath) == false)
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
			logger->DebugFormat("FileRepositoryDB::AddFile() key [%s] exists in multiFile", key.c_str());

		}
	}
	else
	{
		logger->DebugFormat("FileRepositoryDB::AddFile() key [%s] exists", key.c_str());
	}

	return key;
}

bool FileRepositoryDB::GetFile(const std::string& handle, boost::filesystem::path outFilePath)
{
	logger->DebugFormat("FileRepositoryDB::GetFile() handle:[%s] destination:[%s]",
		handle.c_str(),
		outFilePath.string().c_str());

	std::string key = handle;
	boost::filesystem::path srcPath;


	SQLite::Statement query(*db, "SELECT Path FROM Files WHERE DigestValue=:key");
	query.bind(":key", handle);
	bool retValue = false;
	if (this->HasFile(key, &srcPath))
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

	path destPath = this->dataRootPath / boost::filesystem::path(digest);
	boost::filesystem::create_directories(destPath.branch_path());
	if (copy_file_logged(this->multiFile.zipFile, destPath) == false)
	{
		logger->ErrorFormat("FileRepositoryDB::AddFile() key [%s] Failed", key.c_str());
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

std::shared_ptr<IFileRepositoryDB> CreateFileRepositorySQLiteDB(boost::filesystem::path dbPath, boost::filesystem::path dataRootPath)
{
	static auto logger = LoggerFactory::getLogger("application.FileRepositoryDB");
	const long long minFileToBulk = 128 * 1024;
	const long long bulkFileSize = 5 * 1024 * 1024;

	logger->DebugFormat("Creating FileRepositoryDB dbPath:[%s] dataRootPath:[%s] minFileToBulk:[%lld] fileBulkSize:[%lld]", 
		dbPath.string().c_str(), 
		dataRootPath.string().c_str(),
		minFileToBulk,
		bulkFileSize);

	return std::make_shared<FileRepositoryDB>(dbPath, dataRootPath, minFileToBulk, bulkFileSize);
}


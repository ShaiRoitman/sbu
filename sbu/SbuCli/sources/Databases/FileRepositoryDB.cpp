#include "FileRepositoryDB.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "utils.h"

#include "FileRepositoryDBImpl.h"

#include "Poco/Zip/Compress.h"
#include "Poco/Zip/ZipManipulator.h"
#include "Poco/Zip/ZipStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/TemporaryFile.h"


#include <iostream>
#include <ostream>
#include <sstream>

using namespace boost::filesystem;
using namespace SQLite;

ZipWrapper::ZipWrapper(const std::string& fileName) :
	logger(LoggerFactory::getLogger("application.ZipWrapper"))
{
	this->zipArchiveStream = std::make_shared<std::ifstream>(fileName, std::ios::binary);
	this->zipArchiveStreamName = fileName;
	this->zipArchive = std::make_shared<Poco::Zip::ZipArchive>(*this->zipArchiveStream);
	logger->DebugFormat("ZipWrapper::ZipWrapper() using filename:[%s]", fileName);
}
ZipWrapper::~ZipWrapper()
{

}
bool ZipWrapper::ExtractFile(const std::string& handle, const std::string& path)
{
	bool retValue = false;
	logger->DebugFormat("ZipWrapper::ExtractFile() Extract handle:[%s] path:[%s]", handle, path);
	try {
		Poco::Zip::ZipArchive::FileHeaders::const_iterator it = this->zipArchive->findHeader(handle);
		Poco::Zip::ZipInputStream zipin(*this->zipArchiveStream, it->second);
		std::ofstream out(path, std::ios::binary);
		Poco::StreamCopier::copyStream(zipin, out);
		out.close();
		retValue = true;
	}
	catch (std::exception ex)
	{
		logger->ErrorFormat("ZipWrapper::ExtractFile() Extract handle:[%s] path:[%s] Failed exception:[%s]",
			handle, path, ex.what());
	}
	return retValue;
}

bool ZipWrapper::Close()
{
	bool retValue = false;
	if (zipArchiveStream != nullptr)
	{
		zipArchiveStream->close();
		zipArchiveStream = nullptr;
	}

	if (zipArchive != nullptr)
	{
		zipArchive = nullptr;
	}

	logger->DebugFormat("ZipWrapper::Close() filename:[%s] closing:[%d]", zipArchiveStreamName, retValue);
	return retValue;
}


MultiFile::MultiFile() :
	logger(LoggerFactory::getLogger("application.MultiFile"))
{
	this->totalSize = 0;
	this->fileSize = 0;
	this->zipFile = Poco::TemporaryFile::tempName();
	logger->DebugFormat("MultiFile::MultiFile() using filename [%s]", this->zipFile);
	boost::filesystem::remove(this->zipFile);
	zip = nullptr;
}

MultiFile::~MultiFile()
{
	logger->DebugFormat("MultiFile::~MultiFile() closing filename:[%s]", this->zipFile);
	this->Close();
}

Poco::UInt64 MultiFile::GetSize()
{
	auto retValue = this->totalSize;
	logger->DebugFormat("MultiFile::GetSize() filename [%s]:currentSize:[%lld]", this->zipFile, this->totalSize);
	return retValue;
}

bool MultiFile::Close() 
{
	bool retValue = false;
	if (this->zip != nullptr)
	{
		zip->commit();
		zip = nullptr;
		retValue = true;
	}

	logger->DebugFormat("MultiFile::Close() filename:[%s] closing:[%d]", this->zipFile, retValue);
	return retValue;
}

bool MultiFile::AddFile(boost::filesystem::path file, const std::string& digest)
{
	if (this->HasFile(digest))
	{
		logger->DebugFormat("MultiFile::AddFile() filename:[%s], path:[%s] digest:[%s] Already Exists", this->zipFile, file.string(), digest);
		return false;
	}
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
			zip = nullptr;
		}
		zip = std::make_shared<Poco::Zip::ZipManipulator>(this->zipFile, false);
	}
	zip->addFile(digest, file.string());
	this->totalSize += newEntry.size;

	logger->DebugFormat("MultiFile::AddFile() filename:[%s], path:[%s] digest:[%s] Size:[%ld]", this->zipFile, file.string(), digest, newEntry.size);
	return true;
}

bool MultiFile::HasFile(const std::string& digest)
{
	bool retValue = false;
	if (entries.find(digest) != entries.end())
	{
		retValue = true;
	}

	logger->DebugFormat("MultiFile::HasFile() filename [%s], digest [%s] retValue ", this->zipFile, digest, retValue);
	return retValue;
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
	SQLite::Statement query(*db, "SELECT Path,HostDigest FROM Files WHERE DigestValue=:key");
	query.bind(":key", key);
	bool retValue = false;
	if (query.executeStep())
	{
		if (path != nullptr)
		{
			if (query.getColumn("Path").isNull() == false)
			{
				*path = this->dataRootPath / from_utf8(query.getColumn("Path").getString());
			}
			else
			{
				auto hostDigest = query.getColumn("HostDigest").getString();
				if (this->zipFiles.find(hostDigest) == this->zipFiles.end())
				{
					auto hostPath = this->dataRootPath / boost::filesystem::path(hostDigest);
					this->zipFiles[hostDigest] = std::make_shared<ZipWrapper>(hostPath.string());
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
			logger->DebugFormat("FileRepositoryDB::AddFile() key:[%s] exists in multiFile", key.c_str());

		}
	}
	else
	{
		logger->DebugFormat("FileRepositoryDB::AddFile() key:[%s] exists", key.c_str());
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

std::shared_ptr<IFileRepositoryDB> CreateFileRepositorySQLiteDB(boost::filesystem::path dbPath,
	boost::filesystem::path dataRootPath,
	long minSizeToBulk,
	long bulkSize)
{
	static auto logger = LoggerFactory::getLogger("application.FileRepositoryDB");

	logger->DebugFormat("Creating FileRepositoryDB dbPath:[%s] dataRootPath:[%s] minFileToBulk:[%lld] fileBulkSize:[%lld]", 
		dbPath.string().c_str(), 
		dataRootPath.string().c_str(),
		minSizeToBulk,
		bulkSize);

	return std::make_shared<FileRepositoryDB>(dbPath, dataRootPath, minSizeToBulk, bulkSize);
}


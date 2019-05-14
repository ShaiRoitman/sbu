#pragma once

#include <boost/filesystem.hpp>
#include <string>

class IFileRepositoryDB
{
public:
	typedef std::string RepoHandle;
	virtual RepoHandle AddFile(boost::filesystem::path file, const std::string& digestType, const std::string& digest) = 0;
	virtual bool HasFile(const RepoHandle& handle) = 0;
	virtual bool GetFile(const RepoHandle& handle, boost::filesystem::path outFilePath) = 0;
	virtual void Complete() = 0;
};

class IStorageHandler
{
public:
	virtual bool CopyFileToRepository(const IFileRepositoryDB::RepoHandle& handle, boost::filesystem::path srcFilePath) = 0;
	virtual bool CopyFileFromRepository(const IFileRepositoryDB::RepoHandle& handle, boost::filesystem::path dstFilePath) = 0;
};

std::shared_ptr<IFileRepositoryDB> CreateFileRepositorySQLiteDB(
	std::shared_ptr<IStorageHandler> storageHander,
	boost::filesystem::path dbPath,
	long minSizeToBulk,
	long bulkSize);
std::shared_ptr<IFileRepositoryDB> CreateSecureFileRepositorySQLiteDB(
	std::shared_ptr<IStorageHandler> storageHander,
	boost::filesystem::path dbPath,
	const std::string& password,
	long minSizeToBulk,
	long bulkSize);
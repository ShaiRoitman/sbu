#pragma once

#include <boost/filesystem.hpp>
#include <string>

class IFileRepositoryDB
{
public:
	virtual std::string AddFile(boost::filesystem::path file, const std::string& digestType, const std::string& digest) = 0;
	virtual bool HasFile(const std::string& handle) = 0;
	virtual bool GetFile(const std::string& handle, boost::filesystem::path outFilePath) = 0;
	virtual void Complete() = 0;
};

std::shared_ptr<IFileRepositoryDB> CreateFileRepositorySQLiteDB(
	boost::filesystem::path dbPath,
	boost::filesystem::path dataRootPath,
	long minSizeToBulk,
	long bulkSize);
std::shared_ptr<IFileRepositoryDB> CreateSecureFileRepositorySQLiteDB(
	boost::filesystem::path dbPath, 
	boost::filesystem::path dataRootPath, 
	const std::string& password,
	long minSizeToBulk,
	long bulkSize);
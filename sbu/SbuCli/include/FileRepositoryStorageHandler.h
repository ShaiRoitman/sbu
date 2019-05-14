#pragma once
#include "FileRepositoryDB.h"

class FileSystemStorageHandler : public IStorageHandler
{
public:
	FileSystemStorageHandler(boost::filesystem::path dataRootPath);
	virtual bool CopyFileToRepository(const IFileRepositoryDB::RepoHandle& handle, boost::filesystem::path srcFilePath) override;
	virtual bool CopyFileFromRepository(const IFileRepositoryDB::RepoHandle& handle, boost::filesystem::path dstFilePath) override;

public:
	boost::filesystem::path dataRootPath;
};

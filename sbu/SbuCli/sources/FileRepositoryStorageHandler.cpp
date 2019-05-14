#include "FileRepositoryStorageHandler.h"
#include "Loggers.h"
#include "utils.h"

static std::shared_ptr<ILogger> logger = LoggerFactory::getLogger("application.FileSystemStorageHandler");

FileSystemStorageHandler::FileSystemStorageHandler(boost::filesystem::path dataRootPath)
{
	logger->DebugFormat("FileSystemStorageHandler::FileSystemStorageHandler() dataRootPath:[%s]", dataRootPath.string().c_str());
	this->dataRootPath = dataRootPath;
}

bool FileSystemStorageHandler::CopyFileToRepository(const IFileRepositoryDB::RepoHandle& handle, boost::filesystem::path srcFilePath)
{
	auto repoFilePath = this->dataRootPath / handle;
	boost::filesystem::create_directories(repoFilePath.branch_path());
	bool retValue = copy_file_logged(srcFilePath, repoFilePath);
	logger->InfoFormat("FileRepositoryDB::CopyFileToRepository() Handle:[%s] FilePath:[%s]", handle.c_str(), srcFilePath.string().c_str());
	return retValue;
}

bool FileSystemStorageHandler::CopyFileFromRepository(const IFileRepositoryDB::RepoHandle& handle, boost::filesystem::path dstFilePath)
{
	auto repoFilePath = this->dataRootPath / handle;
	boost::filesystem::create_directories(dstFilePath.branch_path());
	bool retValue = copy_file_logged(repoFilePath, dstFilePath);
	logger->InfoFormat("FileRepositoryDB::CopyFileFromRepository() Handle:[%s] FilePath:[%s]", handle.c_str(), dstFilePath.string().c_str());
	return retValue;
}
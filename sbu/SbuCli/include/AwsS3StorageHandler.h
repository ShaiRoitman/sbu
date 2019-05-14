#pragma once
#include "FileRepositoryDB.h"
#include "aws/s3/S3Client.h"
#include "aws/core/utils/threading/Executor.h"
#include "aws/transfer/TransferManager.h"

class AwsS3StorageHandler : public IStorageHandler
{
public:
	AwsS3StorageHandler(
		std::string region,
		std::string bucket,
		std::string basePath,
		std::string key,
		std::string secret
	);
	virtual bool CopyFileToRepository(const IFileRepositoryDB::RepoHandle& handle, boost::filesystem::path srcFilePath) override;
	virtual bool CopyFileFromRepository(const IFileRepositoryDB::RepoHandle& handle, boost::filesystem::path dstFilePath) override;
public:
	std::shared_ptr< Aws::S3::S3Client> client;
	Aws::Utils::Threading::Executor* executor;
	std::shared_ptr<Aws::Transfer::TransferManagerConfiguration> transferConfiguration;
	std::shared_ptr<Aws::Transfer::TransferManager> transferManager;
	std::string basePath;
	std::string bucket;
	std::string key;
	std::string secret;
	std::string region;
};

